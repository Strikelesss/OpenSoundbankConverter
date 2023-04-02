#include "Header/BankConverter.h"
#include "Header/BinaryWriter.h"
#include "Header/E4Sample.h"
#include "Header/VoiceDefinitions.h"
#include "Header/E4Result.h"
#include "Header/BinaryReader.h"
#include "Header/MathFunctions.h"
#include "Header/Logger.h"
#include <fstream>
#include <iostream>

// SF2 reading
#define TSF_NO_STDIO
#define TSF_IMPLEMENTATION
#include "Dependencies/TinySoundFont/tsf.h"

// SF2 writing
#include <sf2cute.hpp>

//
// These are NOT correct but will work until we can determine how each sound in the SF2
// TODO: correct these values

int16_t SF2Converter::filterFreqPercentToCents(const float filterFreq)
{
    if (filterFreq > MAX_FILTER_FREQ_HZ_CORDS || filterFreq < -MAX_FILTER_FREQ_HZ_CORDS)
    {
        Logger::LogMessage("Invalid filter frequency");
        return 0i16;
    }
    return static_cast<int16_t>(MAX_FILTER_FREQ_HZ_CORDS * filterFreq / 100.f);
}

float SF2Converter::centsToFilterFreqPercent(const int16_t cents)
{
    return static_cast<float>(cents) * 100.f / MAX_FILTER_FREQ_HZ_CORDS;
}

//

int16_t SF2Converter::secToTimecent(const double sec)
{
    return static_cast<int16_t>(std::lround(std::log(sec) / std::log(2) * 1200.));
}

bool BankConverter::ConvertE4BToSF2(const E4Result& e4b, const std::string_view& bankName, const ConverterOptions& options) const
{
    if (e4b.GetSamples().empty() || e4b.GetPresets().empty() || bankName.empty())
    {
        Logger::LogMessage("Bank had no samples/presets, or the provided name was empty.");
        return false;
    }

    const auto convertedName(ConvertNameToSFName(bankName));

    sf2cute::SoundFont sf2;
    sf2.set_bank_name(convertedName);
    sf2.set_sound_engine("EMU8000");

    std::string currentPresetStr;
    currentPresetStr.resize(static_cast<size_t>(std::snprintf(nullptr, 0, "Current preset is set to %d", e4b.GetCurrentPreset())) + 1);
    const auto newSize(std::snprintf(currentPresetStr.data(), currentPresetStr.size(), "Current preset is set to %d", e4b.GetCurrentPreset()));

    // Remove the null-terminating character
    currentPresetStr.resize(newSize);

    sf2.set_comment(std::move(currentPresetStr));

    for (const auto& e4Sample : e4b.GetSamples())
    {
        if (e4Sample.GetNumChannels() != 1u)
        {
            Logger::LogMessage("Unable to support stereo samples (sample: %s)", e4Sample.GetName().c_str());
            continue;
        }
        sf2.NewSample(e4Sample.GetName(), e4Sample.GetData(), e4Sample.GetLoopStart(), e4Sample.GetLoopEnd(), e4Sample.GetSampleRate(), 0ui8, 0i8);
    }

    for (const auto& preset : e4b.GetPresets())
    {
        std::vector<sf2cute::SFPresetZone> presetZones;
        std::vector<sf2cute::SFInstrumentZone> instrumentZones;
        for (const auto& voice : preset.GetVoices())
        {
            const auto sampleIndex(e4b.GetSampleIndexMapping().at(voice.GetSampleIndex()));
            const auto& e4Sample(e4b.GetSamples()[sampleIndex]);

            uint16_t sampleMode(0ui16);
            if (e4Sample.IsLooping()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopContinuously); }
            if (e4Sample.IsReleasing()) { sampleMode |= 2ui16; }

            const auto& zoneRange(voice.GetKeyZoneRange());
            const auto& velRange(voice.GetVelocityRange());

            sf2cute::SFInstrumentZone instrumentZone(sf2.samples()[sampleIndex], std::vector{
                sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kKeyRange, sf2cute::RangesType(zoneRange.GetLow(), zoneRange.GetHigh())),
                sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kVelRange, sf2cute::RangesType(velRange.GetLow(), velRange.GetHigh()))
            }, std::vector<sf2cute::SFModulatorItem>{});

            const auto voiceVolBefore(voice.GetVolume());
            const auto voiceVolumeAbs(std::clamp(static_cast<int16_t>(std::abs(voiceVolBefore) * 10i16), 0i16, 144i16)); // Using abs on volume since SF2 does not support negative attenuation
            if (voiceVolumeAbs != 0i16)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialAttenuation, voiceVolumeAbs));
            }

            const auto originalKey(voice.GetOriginalKey());
            if (originalKey != 0ui8)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kOverridingRootKey, originalKey));
            }

            if (sampleMode != 0ui16)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSampleModes, static_cast<int16_t>(sampleMode)));
            }

            // Envelope
            // TODO: plot points from E4B onto an ADSR envelope and grab the time, since the time is inaccurate below (a binary value of 126 could be 2.1 sec, when it normally is say 80 sec)

            const auto& ampEnv(voice.GetAmpEnv());
            const auto ampDelaySec(SF2Converter::secToTimecent(voice.GetKeyDelay()));
            if (ampDelaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayVolEnv, ampDelaySec)); }

            const auto ampAttackSec(SF2Converter::secToTimecent(ampEnv.GetAttack2Sec()));
            if (ampAttackSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackVolEnv, ampAttackSec)); }

            const auto ampHoldSec(SF2Converter::secToTimecent(ampEnv.GetDecay1Sec()));
            if (ampHoldSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kHoldVolEnv, ampHoldSec)); }

            // Sustain Level is expressed in dB for Amp Env, and is also opposite because of SF2
            const auto ampSustainLevel(ampEnv.GetDecay2Level());
            if (ampSustainLevel < 100.f)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainVolEnv,
                    VoiceDefinitions::valueToRelPercent(-(ampSustainLevel / 100.f * SF2Converter::MAX_SUSTAIN_VOL_ENV) + SF2Converter::MAX_SUSTAIN_VOL_ENV)));
            }

            const auto ampDecaySec(SF2Converter::secToTimecent(ampEnv.GetDecay2Sec()));
            if (ampSustainLevel < 100.f && ampDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayVolEnv, ampDecaySec)); }

            const auto ampReleaseSec(SF2Converter::secToTimecent(ampEnv.GetRelease1Sec()));
            if (ampReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseVolEnv, ampReleaseSec)); }

            /*
             * Filter Env
             */

            const auto& filterEnv(voice.GetFilterEnv());
            const auto filterAttackSec(SF2Converter::secToTimecent(filterEnv.GetAttack2Sec()));
            if (filterAttackSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackModEnv, filterAttackSec)); }

            const auto filterDelaySec(SF2Converter::secToTimecent(filterEnv.GetAttack1Sec()));
            if (filterDelaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayModEnv, filterDelaySec)); }

            const auto filterHoldSec(SF2Converter::secToTimecent(filterEnv.GetDecay1Sec()));
            if (filterHoldSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kHoldModEnv, filterHoldSec)); }

            // Opposite because of SF2
            const auto filterSustainLevel(filterEnv.GetDecay2Level());
            if (filterSustainLevel < 100.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainModEnv, VoiceDefinitions::valueToRelPercent(-filterSustainLevel + 100.f))); }

            const auto filterDecaySec(SF2Converter::secToTimecent(filterEnv.GetDecay2Sec()));
            if (filterSustainLevel < 100.f && filterDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayModEnv, filterDecaySec)); }

            const auto filterReleaseSec(SF2Converter::secToTimecent(filterEnv.GetRelease1Sec()));
            if (filterReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseModEnv, filterReleaseSec)); }

            // Filters

            const auto filterFreqCents(VoiceDefinitions::hertzToCents(voice.GetFilterFrequency()));
            if (filterFreqCents >= SF2Converter::SF2_FILTER_MIN_FREQ && filterFreqCents < SF2Converter::SF2_FILTER_MAX_FREQ) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, filterFreqCents)); }

            const auto filterQ(voice.GetFilterQ());
            if (filterQ > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ, VoiceDefinitions::valueToRelPercent(filterQ))); }

            // LFO

            const auto lfo1Freq(VoiceDefinitions::hertzToCents(voice.GetLFO1().GetRate()));
            if (lfo1Freq != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFreqModLFO, lfo1Freq)); }

            const auto lfo1Delay(SF2Converter::secToTimecent(voice.GetLFO1().GetDelay()));
            if (lfo1Delay != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayModLFO, lfo1Delay)); }

            if (options.m_useConverterSpecificData)
            {
                const auto lfo1Shape(voice.GetLFO1().GetShape());
                if (lfo1Shape != 0ui8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused3, lfo1Shape)); }

                const auto lfo1KeySync(voice.GetLFO1().IsKeySync());
                if (lfo1KeySync) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused4, 1i16)); }
            }

            // Cords
            // Unipolar = +
            // Bipolar = ~

            float cordAmtPercent1(0.f);
            if (voice.GetAmountFromCord(LFO1_POLARITY_CENTER, AMP_VOLUME, cordAmtPercent1))
            {
                // LFO 1 ~ -> Amp Volume
                const auto cB(VoiceDefinitions::convert_dB_to_cB(cordAmtPercent1 * SF2Converter::MIN_MAX_LFO1_TO_VOLUME / 100.f)); // Converted to -15, 15
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToVolume, cB));
            }

            float cordAmtPercent2(0.f);
            if (voice.GetAmountFromCord(LFO1_POLARITY_CENTER, PITCH, cordAmtPercent2))
            {
                // LFO 1 ~ -> Pitch
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToPitch, static_cast<int16_t>(cordAmtPercent2)));
            }

            float cordAmtPercent3(0.f);
            if (voice.GetAmountFromCord(LFO1_POLARITY_CENTER, FILTER_FREQ, cordAmtPercent3))
            {
                // LFO 1 ~ -> Filter Frequency
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent3)));
            }

            if (options.m_useConverterSpecificData)
            {
                float cordAmtPercent4(0.f);
                if (voice.GetAmountFromCord(LFO1_POLARITY_CENTER, AMP_PAN, cordAmtPercent4))
                {
                    // LFO 1 ~ -> Amp Pan
                    instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused1, static_cast<int16_t>(cordAmtPercent4)));
                }
            }

            float cordAmtPercent5(0.f);
            if (voice.GetAmountFromCord(FILTER_ENV_POLARITY_POS, FILTER_FREQ, cordAmtPercent5))
            {
                // Filter Env + -> Filter Frequency
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModEnvToFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent5)));
            }

            float cordAmtPercent6(0.f);
            if (voice.GetAmountFromCord(PITCH_WHEEL, PITCH, cordAmtPercent6))
            {
                // Pitch Wheel -> Pitch
                const auto pitchWheel(sf2cute::SFModulator(sf2cute::SFGeneralController::kPitchWheel, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(pitchWheel, sf2cute::SFGenerator(), static_cast<int16_t>(cordAmtPercent6),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent7(0.f);
            if (voice.GetAmountFromCord(MIDI_A, AMP_VOLUME, cordAmtPercent7))
            {
                // MIDI A -> Amp Volume
                const auto midiA(sf2cute::SFModulator(sf2cute::SFMidiController::kController21, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(midiA, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(cordAmtPercent7),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent8(0.f);
            if (voice.GetAmountFromCord(VEL_POLARITY_POS, FILTER_RES, cordAmtPercent8))
            {
                // Velocity + -> 'Filter Resonance
                const auto velPos(sf2cute::SFModulator(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(velPos, sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(cordAmtPercent8),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            const auto velLess(sf2cute::SFModulator(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kDecrease,
                sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

            float cordAmtPercent9(0.f);
            if (voice.GetAmountFromCord(VEL_POLARITY_LESS, AMP_VOLUME, cordAmtPercent9))
            {
                // Velocity < -> Amp Volume
                instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(cordAmtPercent9),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent10(0.f);
            if (voice.GetAmountFromCord(VEL_POLARITY_LESS, FILTER_ENV_ATTACK, cordAmtPercent10))
            {
                // Velocity < -> Filter Env Attack
                instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kAttackModEnv, static_cast<int16_t>(cordAmtPercent10),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent11(0.f);
            if (voice.GetAmountFromCord(VEL_POLARITY_LESS, FILTER_FREQ, cordAmtPercent11))
            {
                // Velocity < -> Filter Freq
                instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent11),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent12(0.f);
            if (voice.GetAmountFromCord(VEL_POLARITY_CENTER, AMP_PAN, cordAmtPercent12))
            {
                // Velocity ~ -> Amp Pan
                const auto velCenter(sf2cute::SFModulator(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(velCenter, sf2cute::SFGenerator::kPan, static_cast<int16_t>(cordAmtPercent12),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent13(0.f);
            if (voice.GetAmountFromCord(KEY_POLARITY_CENTER, FILTER_FREQ, cordAmtPercent13))
            {
                // Key ~ -> Filter Freq
                const auto keyCenter(sf2cute::SFModulator(sf2cute::SFGeneralController::kNoteOnKeyNumber, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(keyCenter, sf2cute::SFGenerator::kInitialFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent13),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent14(0.f);
            if (voice.GetAmountFromCord(MOD_WHEEL, FILTER_FREQ, cordAmtPercent14))
            {
                // Mod Wheel -> Filter Freq
                const auto modWheel(sf2cute::SFModulator(sf2cute::SFMidiController::kModulationDepth, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(modWheel, sf2cute::SFGenerator::kInitialFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent14),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent15(0.f);
            if (voice.GetAmountFromCord(PRESSURE, AMP_ENV_ATTACK, cordAmtPercent15))
            {
                // Pressure -> Amp Env Attack
                const auto pressure(sf2cute::SFModulator(sf2cute::SFGeneralController::kChannelPressure, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(pressure, sf2cute::SFGenerator::kAttackVolEnv, static_cast<int16_t>(cordAmtPercent15),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent16(0.f);
            if (voice.GetAmountFromCord(PEDAL, AMP_VOLUME, cordAmtPercent16))
            {
                // Pedal -> Amp Volume
                const auto pedal(sf2cute::SFModulator(sf2cute::SFMidiController::kController4, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(pedal, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(cordAmtPercent16),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
            }

            float cordAmtPercent17(0.f);
            if (voice.GetAmountFromCord(MOD_WHEEL, CORD_3_AMT, cordAmtPercent17))
            {
                // Mod Wheel -> Cord 3 Amount (Vibrato)
                const auto modWheel(sf2cute::SFModulator(sf2cute::SFMidiController::kModulationDepth, sf2cute::SFControllerDirection::kIncrease,
                    sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

                instrumentZone.SetModulator(sf2cute::SFModulatorItem(modWheel, sf2cute::SFGenerator::kVibLfoToPitch, static_cast<int16_t>(cordAmtPercent17),
                    sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));

                // E4B has 2 LFOs, but in a conversion from Emax II there will only be 1 (we will use that for the vibrato LFO as well as the modulation (filter) LFO)
                if (lfo1Freq != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFreqVibLFO, lfo1Freq)); }
                if (lfo1Delay != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayVibLFO, lfo1Delay)); }
            }

            // Amplifier / Oscillator

            const auto pan(options.m_flipPan ? -voice.GetPan() : voice.GetPan());
            if (pan != 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, VoiceDefinitions::valueToRelPercent(pan))); }

            auto fineTune(voice.GetFineTune());

            // TODO: remove
            if (options.m_useTempFineTune && fineTune > 50.)
            {
                fineTune -= 100.;
                fineTune /= 2.;
            }

            if (fineTune != 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(std::round(fineTune)))); }

            const auto coarseTune(voice.GetCoarseTune());
            if (coarseTune != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kCoarseTune, coarseTune)); }

            const auto chorusSend(voice.GetChorusAmount());
            if (chorusSend > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend, VoiceDefinitions::valueToRelPercent(chorusSend))); }

            // Other

            if (options.m_useConverterSpecificData)
            {
                const auto chorusWidth(voice.GetChorusWidth());
                if (chorusWidth > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused5, VoiceDefinitions::valueToRelPercent(chorusWidth))); }

                const auto attenuationSign(voiceVolBefore > 0 ? 1 : voiceVolBefore < 0 ? -1 : 0);
                if (attenuationSign != 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused2, static_cast<int16_t>(attenuationSign))); }
            }

            instrumentZones.emplace_back(instrumentZone);
        }

        const auto& presetName(preset.GetName());
        presetZones.emplace_back(sf2.NewInstrument(presetName, instrumentZones));
        sf2.NewPreset(presetName, preset.GetIndex(), 0ui16, presetZones);
    }

    try
    {
        auto savePath(options.m_saveFolder);
        if (!savePath.empty() && exists(savePath))
        {
            auto sf2Path(savePath.append(std::filesystem::path(convertedName + ".sf2").wstring()));
            std::ofstream ofs(sf2Path, std::ios::binary);
            sf2.Write(ofs);
            return true;
        }
    }
    catch (const std::fstream::failure& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return false;
}

// Most early Emulator samplers (ex. Emax II) had a maximum of 8MB for each bank
// TODO: account for the 8MB maximum by adding multisample for voices that share the same data
// without multisample: 306 + 306 = 612 bytes
// with multisample: 306 + 44 = 350 bytes
bool BankConverter::ConvertSF2ToE4B(const std::filesystem::path& bank, const std::string_view& bankName, const ConverterOptions& options) const
{
    if (!bank.empty() && !bankName.empty() && exists(bank))
    {
        std::string sf2PathTemp(bankName);
        std::filesystem::path sf2Path;
        auto savePath(options.m_saveFolder);
        if (!savePath.empty() && exists(savePath))
        {
            sf2Path = savePath.append(sf2PathTemp + ".E4B");
        }
        else
        {
            Logger::LogMessage("Path was empty or did not exist.");
            return false;
        }

        BinaryWriter writer(sf2Path);
        BinaryReader reader;
        if (reader.readFile(bank))
        {
            const auto& sf2Data(reader.GetData());
            const tsf* sf2(tsf_load_memory(sf2Data.data(), static_cast<int>(sf2Data.size())));
            if (sf2 != nullptr)
            {
                if (writer.writeType(E4BVariables::EOS_FORM_TAG.data(), E4BVariables::EOS_FORM_TAG.length()))
                {
                    const auto& numPresets(sf2->presetNum);
                    if (numPresets <= 0)
                    {
                        Logger::LogMessage("Preset count was <= 0");
                        return false;
                    }

                    std::unordered_map<size_t, size_t> presetChunkLocations{}, sampleChunkLocations{};

                    // E4B0 + TOC1 + LENGTH
                    auto totalSize(static_cast<uint32_t>(E4BVariables::EOS_E4_FORMAT_TAG.length() + E4BVariables::EOS_TOC_TAG.length() + sizeof(uint32_t)));

                    totalSize += numPresets * (E4BVariables::EOS_CHUNK_TOTAL_LEN + E4BVariables::EOS_CHUNK_NAME_OFFSET + TOTAL_PRESET_DATA_SIZE);

                    uint32_t tocSize(numPresets * E4BVariables::EOS_CHUNK_TOTAL_LEN);

                    const auto& shdrs(sf2->shdrs);
                    for (const auto& shdr : shdrs)
                    {
                        const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
                        if (sampleSize > 0u)
                        {
                            totalSize += E4BVariables::EOS_CHUNK_TOTAL_LEN + E4BVariables::EOS_CHUNK_NAME_OFFSET + TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize;
                            tocSize += E4BVariables::EOS_CHUNK_TOTAL_LEN;
                        }
                    }

                    for (int i(0); i < numPresets; ++i)
                    {
                        const auto& preset(sf2->presets[i]);
                        totalSize += preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE);
                    }

                    totalSize += static_cast<uint32_t>(E4BVariables::EOS_EMSt_TAG.length());
                    totalSize += TOTAL_EMST_DATA_SIZE;

                    // Byteswap since E4B requires it
                    totalSize = _byteswap_ulong(totalSize);
                    tocSize = _byteswap_ulong(tocSize);

                    if (writer.writeType(&totalSize))
                    {
                        if (writer.writeType(E4BVariables::EOS_E4_FORMAT_TAG.data(), E4BVariables::EOS_E4_FORMAT_TAG.length()))
                        {
                            if (writer.writeType(E4BVariables::EOS_TOC_TAG.data(), E4BVariables::EOS_TOC_TAG.length()))
                            {
                                if (writer.writeType(&tocSize))
                                {
                                    constexpr uint16_t redundantChunkVal(0ui16);

                                    /*
                                     * Chunks
                                     */

                                    for (int i(0); i < numPresets; ++i)
                                    {
                                        if (writer.writeType(E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()))
                                        {
                                            const auto& preset(sf2->presets[i]);
                                            const uint32_t presetChunkLen(_byteswap_ulong(TOTAL_PRESET_DATA_SIZE + preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
                                            if (writer.writeType(&presetChunkLen))
                                            {
                                                const auto presetIndex(preset.preset);
                                                presetChunkLocations[presetIndex] = writer.GetWritePos();

                                                // Set to 0 since we go back to it below
                                                constexpr uint32_t presetChunkLoc(0u);
                                                if (writer.writeType(&presetChunkLoc))
                                                {
                                                    const auto presetIndexBS(_byteswap_ushort(presetIndex));
                                                    if (writer.writeType(&presetIndexBS) && writer.writeType(ConvertNameToEmuName(preset.presetName.data()).c_str(),
                                                        E4BVariables::EOS_E4_MAX_NAME_LEN) && writer.writeType(&redundantChunkVal)) { continue; }
                                                }
                                            }
                                        }

                                        Logger::LogMessage("Failed to write preset chunk!");
                                        return false;
                                    }

                                    uint16_t sampleIndex(0ui16);
                                    for (const auto& shdr : shdrs)
                                    {
                                        const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
                                        if (sampleSize > 0u)
                                        {
                                            if (writer.writeType(E4BVariables::EOS_E3_SAMPLE_TAG.data(), E4BVariables::EOS_E3_SAMPLE_TAG.length()))
                                            {
                                                const uint32_t sampleChunkLen(_byteswap_ulong(TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize));
                                                if (writer.writeType(&sampleChunkLen))
                                                {
                                                    sampleChunkLocations[sampleIndex] = writer.GetWritePos();

                                                    // Set to 0 since we go back to it below
                                                    constexpr uint32_t sampleChunkLoc(0u);
                                                    if (writer.writeType(&sampleChunkLoc))
                                                    {
                                                        const auto sampleIndexBS(_byteswap_ushort(sampleIndex));
                                                        if (writer.writeType(&sampleIndexBS) && writer.writeType(ConvertNameToEmuName(shdr.sampleName.data()).c_str(),
                                                            E4BVariables::EOS_E4_MAX_NAME_LEN) && writer.writeType(&redundantChunkVal))
                                                        {
                                                            ++sampleIndex;
                                                            continue;
                                                        }
                                                    }
                                                }
                                            }

                                            Logger::LogMessage("Failed to write sample chunk!");
                                            return false;
                                        }
                                    }

                                    /*
                                     * Data
                                     */

                                    std::unordered_map<uint8_t, uint8_t> sampleModes{};

                                    for (int i(0); i < numPresets; ++i)
                                    {
                                        const auto& preset(sf2->presets[i]);
                                        const auto presetIndex(preset.preset);
                                        const auto currentPos(_byteswap_ulong(static_cast<uint32_t>(writer.GetWritePos())));
                                        if (writer.writeTypeAtLocation(&currentPos, presetChunkLocations[presetIndex]) && writer.writeType(E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()))
                                        {
                                            uint32_t presetChunkLen(_byteswap_ulong(sizeof(uint16_t) + TOTAL_PRESET_DATA_SIZE + preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
                                            if (writer.writeType(&presetChunkLen))
                                            {
                                                const auto presetNum(_byteswap_ushort(presetIndex));
                                                if (writer.writeType(&presetNum) && writer.writeType(ConvertNameToEmuName(preset.presetName.data()).c_str(), E4BVariables::EOS_E4_MAX_NAME_LEN))
                                                {
                                                    const auto presetDataSize(_byteswap_ushort(static_cast<uint16_t>(TOTAL_PRESET_DATA_SIZE)));
                                                    if (writer.writeType(&presetDataSize))
                                                    {
                                                        const auto numVoices(static_cast<uint16_t>(preset.regionNum));
                                                        const auto numVoicesBS(_byteswap_ushort(numVoices));
                                                        if (writer.writeType(&numVoicesBS))
                                                        {
                                                            constexpr std::array<int8_t, 30> redundantPresetData1{};
                                                            if (writer.writeType(redundantPresetData1.data(), sizeof(int8_t) * redundantPresetData1.size()))
                                                            {
                                                                constexpr std::array<uint8_t, 8> redundantPresetData2{'R', '#', '\0', '~', 255ui8, 255ui8, 255ui8, 255ui8};
                                                                if (writer.writeType(redundantPresetData2.data(), sizeof(uint8_t) * redundantPresetData2.size()))
                                                                {
                                                                    constexpr std::array<int8_t, 24> redundantPresetData3{};
                                                                    if (writer.writeType(redundantPresetData3.data(), sizeof(int8_t) * redundantPresetData3.size()))
                                                                    {
                                                                        for (uint16_t j(0ui16); j < numVoices; ++j)
                                                                        {
                                                                            const auto& region(preset.regions[j]);
                                                                            const auto filterFreq(static_cast<int16_t>(tsf_cents2Hertz(static_cast<float>(region.initialFilterFc))));
                                                                            const auto convertedPan(std::round(VoiceDefinitions::relPercentToValue(region.pan)));
                                                                            const auto pan(static_cast<int8_t>(options.m_flipPan ? -convertedPan : convertedPan));

                                                                            auto volume(static_cast<int8_t>(region.attenuation));
                                                                            if (options.m_useConverterSpecificData)
                                                                            {
                                                                                // Multiply by the sign since SF2 does not support negative attenuation
                                                                                const auto attenuationSign(region.unused2);
                                                                                volume = static_cast<int8_t>(attenuationSign != 0
                                                                                                                 ? region.attenuation *
                                                                                                                 static_cast<float>(attenuationSign)
                                                                                                                 : region.attenuation);
                                                                            }

                                                                            const auto fineTune(static_cast<double>(region.tune));
                                                                            const auto coarseTune(static_cast<int8_t>(region.transpose));
                                                                            const auto filterQ(static_cast<float>(region.initialFilterQ) / 10.f);

                                                                            const auto& ampEnv(region.ampenv);
                                                                            const auto keyDelay(MathFunctions::round_f_places(ampEnv.delay, 3u));

                                                                            const auto ampAttack(static_cast<double>(ampEnv.attack));
                                                                            const auto ampRelease(static_cast<double>(ampEnv.release));
                                                                            const auto ampDecay(static_cast<double>(ampEnv.decay));
                                                                            const auto ampHold(static_cast<double>(ampEnv.hold));
                                                                            const auto ampSustain(ampEnv.sustain * 100.f);

                                                                            const auto& filterEnv(region.modenv);
                                                                            const auto filterAttack(static_cast<double>(filterEnv.attack));
                                                                            const auto filterRelease(static_cast<double>(filterEnv.release));
                                                                            const auto filterDelay(static_cast<double>(filterEnv.delay));
                                                                            const auto filterDecay(static_cast<double>(filterEnv.decay));
                                                                            const auto filterHold(static_cast<double>(filterEnv.hold));
                                                                            const auto filterSustain(filterEnv.sustain * 100.f);

                                                                            float chorusAmount(VoiceDefinitions::relPercentToValue(region.chorusEffectsSend));
                                                                            auto chorusWidth(0.f);
                                                                            const auto LFO1Freq(VoiceDefinitions::centsToHertz(static_cast<int16_t>(region.freqModLFO)));
                                                                            const auto LFO1Delay(static_cast<double>(region.delayModLFO));

                                                                            E4LFO lfo1;
                                                                            if (options.m_useConverterSpecificData)
                                                                            {
                                                                                lfo1 = E4LFO(LFO1Freq, static_cast<uint8_t>(region.unused3), LFO1Delay, static_cast<bool>(region.unused4));
                                                                                chorusWidth = VoiceDefinitions::relPercentToValue(region.unused5);
                                                                            }
                                                                            else
                                                                            {
                                                                                lfo1 = E4LFO(LFO1Freq, 0ui8, LFO1Delay, true);
                                                                            }

                                                                            E4Voice voice(chorusWidth, chorusAmount, filterFreq, coarseTune, pan, volume, fineTune, keyDelay, filterQ, {region.lokey, region.hikey},
                                                                                {region.lovel, region.hivel}, E4Envelope(ampAttack, ampDecay, ampHold, ampRelease, 0., ampSustain),
                                                                                E4Envelope(filterAttack, filterDecay, filterHold, filterRelease, filterDelay, filterSustain), lfo1);

                                                                            /*
                                                                             * Cords
                                                                             */

                                                                            voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, PITCH, static_cast<int8_t>(region.modLfoToPitch));

                                                                            const auto modEnvToFilterFreq(static_cast<int16_t>(region.modEnvToFilterFc));
                                                                            voice.ReplaceOrAddCord(FILTER_ENV_POLARITY_POS, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
                                                                                SF2Converter::centsToFilterFreqPercent(modEnvToFilterFreq)));

                                                                            const auto modLfoToVolume(static_cast<int16_t>(region.modLfoToVolume));
                                                                            if (modLfoToVolume != 0i16)
                                                                            {
                                                                                const auto dB(VoiceDefinitions::convert_cB_to_dB(modLfoToVolume));
                                                                                voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(dB * 100.f / SF2Converter::MIN_MAX_LFO1_TO_VOLUME));
                                                                            }

                                                                            const auto modLfoToFilterFc(static_cast<int16_t>(region.modLfoToFilterFc));
                                                                            if (modLfoToFilterFc != 0i16) { voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, FILTER_FREQ, SF2Converter::centsToFilterFreqPercent(modLfoToFilterFc)); }

                                                                            if (options.m_useConverterSpecificData)
                                                                            {
                                                                                const auto LFO1ToAmpPan(static_cast<int8_t>(region.unused1));
                                                                                if (LFO1ToAmpPan != 0i8) { voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, AMP_PAN, VoiceDefinitions::ConvertPercentToByteF(LFO1ToAmpPan)); }
                                                                            }

                                                                            // Default values in E4B
                                                                            bool hasPitchWheel(false);
                                                                            bool hasModWheel(false);

                                                                            for (const auto& mod : region.modulators)
                                                                            {
                                                                                const sf2cute::SFModulator srcOper(mod.modSrcOper);
                                                                                const auto modAmountF(static_cast<float>(mod.modAmount));
                                                                                const auto destOper(static_cast<sf2cute::SFGenerator>(mod.modDestOper));
                                                                                if (mod.modAmount != 0i16)
                                                                                {
                                                                                    if (srcOper.controller_palette() == sf2cute::SFControllerPalette::kGeneralController)
                                                                                    {
                                                                                        if (srcOper.general_controller() == sf2cute::SFGeneralController::kPitchWheel)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator())
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(PITCH_WHEEL, PITCH, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                        hasPitchWheel = true;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        else if (srcOper.general_controller() == sf2cute::SFGeneralController::kNoteOnVelocity)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialFilterQ)
                                                                                                {
                                                                                                    voice.ReplaceOrAddCord(VEL_POLARITY_POS, FILTER_RES, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                }
                                                                                                else if (destOper == sf2cute::SFGenerator::kPan)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(VEL_POLARITY_CENTER, AMP_PAN, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                            else if (srcOper.direction() == sf2cute::SFControllerDirection::kDecrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialAttenuation)
                                                                                                {
                                                                                                    voice.ReplaceOrAddCord(VEL_POLARITY_LESS, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                }
                                                                                                else if (destOper == sf2cute::SFGenerator::kAttackModEnv)
                                                                                                {
                                                                                                    voice.ReplaceOrAddCord(VEL_POLARITY_LESS, FILTER_ENV_ATTACK, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                }
                                                                                                else if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
                                                                                                {
                                                                                                    voice.ReplaceOrAddCord(VEL_POLARITY_LESS, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
                                                                                                        SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        else if (srcOper.general_controller() == sf2cute::SFGeneralController::kNoteOnKeyNumber)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(KEY_POLARITY_CENTER, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
                                                                                                            SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        else if (srcOper.general_controller() == sf2cute::SFGeneralController::kChannelPressure)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kAttackVolEnv)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(PRESSURE, AMP_ENV_ATTACK, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else if (srcOper.controller_palette() == sf2cute::SFControllerPalette::kMidiController)
                                                                                    {
                                                                                        if (srcOper.midi_controller() == sf2cute::SFMidiController::kController21)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialAttenuation)
                                                                                                {
                                                                                                    voice.ReplaceOrAddCord(MIDI_A, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        else if (srcOper.midi_controller() == sf2cute::SFMidiController::kModulationDepth)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(MOD_WHEEL, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
                                                                                                            SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
                                                                                                    }
                                                                                                }
                                                                                                else if(destOper == sf2cute::SFGenerator::kVibLfoToPitch)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(MOD_WHEEL, CORD_3_AMT, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                        hasModWheel = true;
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        else if (srcOper.midi_controller() == sf2cute::SFMidiController::kController4)
                                                                                        {
                                                                                            if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                                                                            {
                                                                                                if (destOper == sf2cute::SFGenerator::kInitialAttenuation)
                                                                                                {
                                                                                                    if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                                                                                    {
                                                                                                        voice.ReplaceOrAddCord(PEDAL, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }

                                                                            if(!hasPitchWheel)
                                                                            {
                                                                                voice.DisableCord(PITCH_WHEEL, PITCH);
                                                                            }

                                                                            if(!hasModWheel)
                                                                            {
                                                                                voice.DisableCord(MOD_WHEEL, CORD_3_AMT);
                                                                            }

                                                                            const auto originalKey(static_cast<uint8_t>(region.pitch_keycenter));
                                                                            E4VoiceEndData voiceEnd(region.sampleIndex, originalKey);

                                                                            if (!sampleModes.contains(region.sampleIndex)) { sampleModes[region.sampleIndex] = static_cast<uint8_t>(region.loop_mode); }

                                                                            if (voice.write(writer) && voiceEnd.write(writer)) { continue; }

                                                                            Logger::LogMessage("Failed to write voice data!");
                                                                            return false;
                                                                        }

                                                                        continue;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        Logger::LogMessage("Failed to write preset data!");
                                        return false;
                                    }

                                    sampleIndex = 0ui16;

                                    // Indexes to keep to avoid non-mono samples from being re-added.
                                    std::vector<uint16_t> completedSampleIndexes;
                                    
                                    for (const auto& shdr : shdrs)
                                    {
                                    	if(shdr.start == 0 && shdr.end == 0)
                                    	{
                                    		Logger::LogMessage("Skipped sample with a start of 0 and end of 0.");
                                    		continue;
                                    	}
                                    	
                                        if(std::ranges::find(completedSampleIndexes, sampleIndex) != completedSampleIndexes.end())
                                        {
                                            ++sampleIndex;
                                            continue;
                                        }
                                        
                                        const auto sampleType(static_cast<sf2cute::SFSampleLink>(shdr.sampleType));
                                        
                                        if(sampleType != sf2cute::SFSampleLink::kMonoSample)
                                        {
                                            Logger::LogMessage("Sample type is unsupported!");
                                            return false;
                                        } 
                                        
                                        const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
                                        if (sampleSize > 0u)
                                        {
                                            const auto currentPos(_byteswap_ulong(static_cast<uint32_t>(writer.GetWritePos())));
                                            if (writer.writeTypeAtLocation(&currentPos, sampleChunkLocations[sampleIndex]) && writer.writeType(E4BVariables::EOS_E3_SAMPLE_TAG.data(), E4BVariables::EOS_E3_SAMPLE_TAG.length()))
                                            {
                                                const uint32_t sampleChunkLen(_byteswap_ulong(sizeof(uint16_t) + TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize));
                                                if (writer.writeType(&sampleChunkLen))
                                                {
                                                    const auto sampleIndexBS(_byteswap_ushort(sampleIndex));
                                                    if (writer.writeType(&sampleIndexBS) && writer.writeType(ConvertNameToEmuName(shdr.sampleName.data()).c_str(), E4BVariables::EOS_E4_MAX_NAME_LEN))
                                                    {
                                                        const uint32_t lastSampleLeft(sampleSize - 2ull + TOTAL_SAMPLE_DATA_READ_SIZE);

                                                        uint32_t lastSampleRight(0u);
                                                        if (lastSampleLeft > TOTAL_SAMPLE_DATA_READ_SIZE)
                                                        {
                                                            lastSampleRight = lastSampleLeft - TOTAL_SAMPLE_DATA_READ_SIZE;
                                                        }

                                                        uint32_t loopStart(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        if (shdr.startLoop > shdr.start)
                                                        {
                                                            loopStart = (shdr.startLoop - shdr.start) * 2u + static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        }

                                                        uint32_t loopEnd(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        if (shdr.endLoop > shdr.start)
                                                        {
                                                            loopEnd = (shdr.endLoop - shdr.start) * 2u + static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        }

                                                        uint32_t loopStart2(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        if (loopStart > TOTAL_SAMPLE_DATA_READ_SIZE)
                                                        {
                                                            loopStart2 = loopStart - TOTAL_SAMPLE_DATA_READ_SIZE;
                                                        }

                                                        uint32_t loopEnd2(TOTAL_SAMPLE_DATA_READ_SIZE);
                                                        if (loopEnd > TOTAL_SAMPLE_DATA_READ_SIZE)
                                                        {
                                                            loopEnd2 = loopEnd - TOTAL_SAMPLE_DATA_READ_SIZE;
                                                        }

                                                        // TODO: account for stereo in param 3
                                                        // and then '0u' in param 1 (seems to vary)

                                                        const std::array params1{
                                                            0u, static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE), 0u,
                                                            lastSampleLeft, lastSampleRight, loopStart, loopStart2, loopEnd, loopEnd2
                                                        };

                                                        if (writer.writeType(params1.data(), sizeof(uint32_t) * params1.size()))
                                                        {
                                                            if (writer.writeType(&shdr.sampleRate))
                                                            {
                                                                // TODO: Support for stereo

                                                                uint32_t format(sampleType == sf2cute::SFSampleLink::kMonoSample ? E4SampleVariables::EOS_MONO_SAMPLE
                                                                    : E4SampleVariables::EOS_STEREO_SAMPLE);

                                                                const auto& mode(sampleModes[static_cast<uint8_t>(sampleIndex)]);
                                                                if (mode == TSF_LOOPMODE_CONTINUOUS)
                                                                {
                                                                    format |= E4SampleVariables::SAMPLE_LOOP_FLAG;
                                                                }
                                                                else if (mode == TSF_LOOPMODE_SUSTAIN)
                                                                {
                                                                    format |= E4SampleVariables::SAMPLE_RELEASE_FLAG;
                                                                }
                                                                else if (mode == TSF_LOOPMODE_CONTINUOUS_SUSTAIN)
                                                                {
                                                                    format |= E4SampleVariables::SAMPLE_RELEASE_FLAG | E4SampleVariables::SAMPLE_LOOP_FLAG;
                                                                }
                                                                
                                                                if(sampleType == sf2cute::SFSampleLink::kMonoSample)
                                                                {
                                                                    if (writer.writeType(&format))
                                                                    {
                                                                        // Generally always empty, can keep as constexpr
                                                                        constexpr std::array<uint32_t, E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS> params2{};
                                                                        if (writer.writeType(params2.data(), sizeof(uint32_t) * params2.size()))
                                                                        {
                                                                            const std::vector sampleData(&sf2->samplesAsShort[shdr.start], &sf2->samplesAsShort[shdr.end]);
                                                                            if (writer.writeType(sampleData.data(), sizeof(uint16_t) * sampleData.size()))
                                                                            {
                                                                                ++sampleIndex;
                                                                                continue;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    if(sampleType == sf2cute::SFSampleLink::kLeftSample)
                                                                    {
                                                                        const auto& rightSampleShdr(shdrs[shdr.sampleLink]);
                                                                        completedSampleIndexes.emplace_back(shdr.sampleLink);
                                                                        completedSampleIndexes.emplace_back(rightSampleShdr.sampleLink);

                                                                        if (writer.writeType(&format))
                                                                        {
                                                                            // Generally always empty, can keep as constexpr
                                                                            constexpr std::array<uint32_t, E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS> params2{};
                                                                            if (writer.writeType(params2.data(), sizeof(uint32_t) * params2.size()))
                                                                            {
                                                                                const std::vector leftSampleData(&sf2->samplesAsShort[shdr.start], &sf2->samplesAsShort[shdr.end]);
                                                                                const std::vector rightSampleData(&sf2->samplesAsShort[rightSampleShdr.start], &sf2->samplesAsShort[rightSampleShdr.end]);

                                                                                std::vector<int16_t> stereoData(leftSampleData.size() + rightSampleData.size());
                                                                                InterleaveSamples(leftSampleData.data(), rightSampleData.data(), stereoData.data(), leftSampleData.size());
                                                                                
                                                                                if (writer.writeType(stereoData.data(), sizeof(uint16_t) * stereoData.size()))
                                                                                {
                                                                                    ++sampleIndex;
                                                                                    continue;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    else if(sampleType == sf2cute::SFSampleLink::kRightSample)
                                                                    {
                                                                        const auto& leftSampleShdr(shdrs[shdr.sampleLink]);
                                                                        completedSampleIndexes.emplace_back(shdr.sampleLink);
                                                                        completedSampleIndexes.emplace_back(leftSampleShdr.sampleLink);

                                                                        if (writer.writeType(&format))
                                                                        {
                                                                            // Generally always empty, can keep as constexpr
                                                                            constexpr std::array<uint32_t, E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS> params2{};
                                                                            if (writer.writeType(params2.data(), sizeof(uint32_t) * params2.size()))
                                                                            {
                                                                                const std::vector leftSampleData(&sf2->samplesAsShort[leftSampleShdr.start], &sf2->samplesAsShort[leftSampleShdr.end]);
                                                                                const std::vector rightSampleData(&sf2->samplesAsShort[shdr.start], &sf2->samplesAsShort[shdr.end]);
                                                                                
                                                                                std::vector<int16_t> stereoData(leftSampleData.size() + rightSampleData.size());
                                                                                InterleaveSamples(leftSampleData.data(), rightSampleData.data(), stereoData.data(), leftSampleData.size());
                                                                                
                                                                                if (writer.writeType(stereoData.data(), sizeof(uint16_t) * stereoData.size()))
                                                                                {
                                                                                    ++sampleIndex;
                                                                                    continue;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        Logger::LogMessage("Sample type is unsupported!");
                                                                        return false;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            Logger::LogMessage("Failed to write sample data!");
                                            return false;
                                        }
                                    }

                                    E4EMSt emst;
                                    if (!writer.writeType(E4BVariables::EOS_EMSt_TAG.data(), E4BVariables::EOS_EMSt_TAG.length()) || !emst.write(writer))
                                    {
                                        Logger::LogMessage("Failed to write EMST data!");
                                        return false;
                                    }
                                }
                            }
                        }

                        return writer.finishWriting();
                    }
                }
            }
        }
    }

    Logger::LogMessage("Provided bank name was empty, or provided path was empty/did not exist.");
    return false;
}

std::string BankConverter::ConvertNameToEmuName(const std::string_view& name) const
{
    std::string str(std::begin(name), std::ranges::find(name, '\0'));
    if (name.length() > E4BVariables::EOS_E4_MAX_NAME_LEN) { str.resize(E4BVariables::EOS_E4_MAX_NAME_LEN); }

    if (str.length() < E4BVariables::EOS_E4_MAX_NAME_LEN)
    {
        for (size_t i(str.length()); i < E4BVariables::EOS_E4_MAX_NAME_LEN; ++i)
        {
            str.append(" ");
        }
    }

    return str;
}

std::string BankConverter::ConvertNameToSFName(const std::string_view& name) const
{
    std::string str(std::begin(name), std::ranges::find(name, '\0'));
    if (name.length() > SF2Converter::SF2_MAX_NAME_LEN) { str.resize(SF2Converter::SF2_MAX_NAME_LEN); }
    return str;
}

void BankConverter::InterleaveSamples(const int16_t* leftChannel, const int16_t* rightChannel, int16_t* outStereo, const size_t sampleNum) const
{
    for (auto i(0); i < sampleNum; ++i)
    {
        outStereo[i * 2] = leftChannel[i];
        outStereo[i * 2 + 1] = rightChannel[i];
    }
}