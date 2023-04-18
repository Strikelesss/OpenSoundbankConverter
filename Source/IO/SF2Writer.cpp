#include "Header/IO/SF2Writer.h"
#include "Header/Data/Soundbank.h"
#include "Header/Logger.h"
#include "Header/MathFunctions.h"
#include "Header/SF2/Helpers/SF2Helpers.h"
#include "Header/BankWriteOptions.h"
#include <filesystem>
#include <cassert>
#include <fstream>

bool SF2Writer::WriteData(const Soundbank& soundbank, const BankWriteOptions& options) const
{
    const std::string convertedName(ConvertNameToSFName(soundbank.m_bankName));

    sf2cute::SoundFont sf2;
    sf2.set_bank_name(convertedName);
    sf2.set_sound_engine("EMU8000");
    sf2.set_comment(std::format("Current preset is set to {0}", soundbank.m_defaultPreset));

    for (const auto& sample : soundbank.m_samples)
    {
        if (sample.m_channels != 1u)
        {
            Logger::LogMessage("Unable to support stereo samples (sample: %s)", sample.m_sampleName.c_str());
            continue;
        }
        
        sf2.NewSample(sample.m_sampleName, sample.m_sampleData, sample.m_loopStart,
            sample.m_loopEnd, sample.m_sampleRate, 0ui8, 0i8);
    }

    for (const auto& preset : soundbank.m_presets)
    {
        std::vector<sf2cute::SFPresetZone> presetZones;
        std::vector<sf2cute::SFInstrumentZone> instrumentZones;
        for (const auto& voice : preset.m_voices)
        {
            // Skip writing voices that have no sample index / banks that have no samples
            if(soundbank.m_samples.empty() || voice.m_sampleIndex <= 0ui8)
            {
                continue;
            }

            const size_t sampleIndex(voice.m_sampleIndex - 1ui8);
            const auto& sample(soundbank.m_samples.at(sampleIndex));

            uint16_t sampleMode(0ui16);
            if (sample.m_isLooping) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopContinuously); }
            if (sample.m_isLoopReleasing) { sampleMode |= 2ui16; }

            const auto& zoneRange(voice.m_keyZone);
            const auto& velRange(voice.m_velocityZone);

            sf2cute::SFInstrumentZone instrumentZone(sf2.samples()[sampleIndex], std::vector{
                sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kKeyRange, sf2cute::RangesType(zoneRange.m_low, zoneRange.m_high)),
                sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kVelRange, sf2cute::RangesType(velRange.m_low, velRange.m_high))
            }, std::vector<sf2cute::SFModulatorItem>{});

            const int8_t voiceVolBefore(voice.m_volume);
            const int16_t voiceVolumeAbs(std::clamp(static_cast<int16_t>(std::abs(voiceVolBefore) * 10i16), 0i16, 144i16)); // Using abs on volume since SF2 does not support negative attenuation
            if (voiceVolumeAbs != 0i16)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialAttenuation, voiceVolumeAbs));
            }
            
            if (voice.m_originalKey != 0ui8)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kOverridingRootKey, voice.m_originalKey));
            }

            if (sampleMode != 0ui16)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSampleModes, static_cast<int16_t>(sampleMode)));
            }

            // Envelope
            // TODO: Plot points from E4B onto an ADSR envelope and grab the time, since the time is inaccurate below (a binary value of 126 could be 2.1 sec, when it normally is say 80 sec)

            const auto& ampEnv(voice.m_ampEnv);
            const int16_t ampDelaySec(SF2Helpers::secToTimecent(ampEnv.m_delaySec));
            if (ampDelaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayVolEnv, ampDelaySec)); }

            const int16_t ampAttackSec(SF2Helpers::secToTimecent(ampEnv.m_attackSec));
            if (ampAttackSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackVolEnv, ampAttackSec)); }

            const int16_t ampHoldSec(SF2Helpers::secToTimecent(ampEnv.m_holdSec));
            if (ampHoldSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kHoldVolEnv, ampHoldSec)); }

            // Sustain Level is expressed in dB for Amp Env, and is also opposite because of SF2
            const float ampSustainLevel(ampEnv.m_sustainDB);
            if (ampSustainLevel < 100.f)
            {
                instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainVolEnv,
                    SF2Helpers::valueToRelativePercent(-(ampSustainLevel / 100.f * SF2Helpers::MAX_SUSTAIN_VOL_ENV) + SF2Helpers::MAX_SUSTAIN_VOL_ENV)));
            }

            const int16_t ampDecaySec(SF2Helpers::secToTimecent(ampEnv.m_decaySec));
            if (ampSustainLevel < 100.f && ampDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayVolEnv, ampDecaySec)); }

            const int16_t ampReleaseSec(SF2Helpers::secToTimecent(ampEnv.m_releaseSec));
            if (ampReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseVolEnv, ampReleaseSec)); }

            /*
             * Filter Env
             */

            const auto& filterEnv(voice.m_filterEnv);
            const int16_t filterAttackSec(SF2Helpers::secToTimecent(filterEnv.m_attackSec));
            if (filterAttackSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackModEnv, filterAttackSec)); }

            const int16_t filterDelaySec(SF2Helpers::secToTimecent(filterEnv.m_delaySec));
            if (filterDelaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayModEnv, filterDelaySec)); }

            const int16_t filterHoldSec(SF2Helpers::secToTimecent(filterEnv.m_holdSec));
            if (filterHoldSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kHoldModEnv, filterHoldSec)); }

            // Opposite because of SF2
            const float filterSustainLevel(filterEnv.m_sustainDB);
            if (filterSustainLevel < 100.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainModEnv,
                SF2Helpers::valueToRelativePercent(-filterSustainLevel + 100.f))); }

            const int16_t filterDecaySec(SF2Helpers::secToTimecent(filterEnv.m_decaySec));
            if (filterSustainLevel < 100.f && filterDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayModEnv, filterDecaySec)); }

            const int16_t filterReleaseSec(SF2Helpers::secToTimecent(filterEnv.m_releaseSec));
            if (filterReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseModEnv, filterReleaseSec)); }

            // Filters

            const int16_t filterFreqCents(SF2Helpers::hertzToCents(voice.m_filterFrequency));
            if (filterFreqCents >= SF2Helpers::SF2_FILTER_MIN_FREQ && filterFreqCents < SF2Helpers::SF2_FILTER_MAX_FREQ) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, filterFreqCents)); }
            
            if (voice.m_filterQ > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ,
                SF2Helpers::valueToRelativePercent(voice.m_filterQ))); }

            // LFO

            const int16_t lfo1Freq(SF2Helpers::hertzToCents(voice.m_lfo1.m_rate));
            if (lfo1Freq != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFreqModLFO, lfo1Freq)); }

            const int16_t lfo1Delay(SF2Helpers::secToTimecent(voice.m_lfo1.m_delay));
            if (lfo1Delay != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayModLFO, lfo1Delay)); }

            if (options.m_useConverterSpecificData)
            {
                const uint8_t lfo1Shape(voice.m_lfo1.m_shape);
                if (lfo1Shape != 0ui8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused3, lfo1Shape)); }

                const bool lfo1KeySync(voice.m_lfo1.m_keySync);
                if (lfo1KeySync) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused4, 1i16)); }
            }

            // Realtime Controls

            for(const auto& rtControl : voice.m_realtimeControls)
            {
                WriteModOrGen(instrumentZone, rtControl, options);
            }
            
            // Amplifier / Oscillator
            
            if (voice.m_pan != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, SF2Helpers::valueToRelativePercent(voice.m_pan))); }
            if (voice.m_fineTune != 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(std::round(voice.m_fineTune)))); }
            if (voice.m_coarseTune != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kCoarseTune, voice.m_coarseTune)); }
            
            if (voice.m_chorusAmount > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend,
                SF2Helpers::valueToRelativePercent(voice.m_chorusAmount))); }

            // Other

            if (options.m_useConverterSpecificData)
            {
                if (voice.m_chorusWidth > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused5,
                    SF2Helpers::valueToRelativePercent(voice.m_chorusWidth))); }

                const int32_t attenuationSign(voiceVolBefore > 0i8 ? 1 : voiceVolBefore < 0i8 ? -1 : 0);
                if (attenuationSign != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused2,
                    static_cast<int16_t>(attenuationSign))); }
            }

            instrumentZones.emplace_back(instrumentZone);
        }

        const auto& presetName(preset.m_presetName);
        presetZones.emplace_back(sf2.NewInstrument(presetName, instrumentZones));
        sf2.NewPreset(presetName, preset.m_index, 0ui16, presetZones);
    }

    try
    {
        auto savePath(options.m_saveFolder);
        if (!savePath.empty() && std::filesystem::exists(savePath))
        {
            auto sf2Path(savePath.append(std::filesystem::path(convertedName + ".sf2").wstring()));
            std::ofstream ofs(sf2Path, std::ios::binary);
            sf2.Write(ofs);
            return true;
        }
    }
    catch (const std::fstream::failure& e)
    {
        Logger::LogMessage(e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        Logger::LogMessage(e.what());
        return false;
    }

    return false;
}

void SF2Writer::WriteModOrGen(sf2cute::SFInstrumentZone& instrumentZone, const BankRealtimeControl& rtControl, const BankWriteOptions& options) const
{
    // Don't write null controls
    if(rtControl.m_src == ERealtimeControlSrc::SRC_OFF && rtControl.m_dst == ERealtimeControlDst::DST_OFF)
    {
        return;
    }

    // Don't write if the control amount is 0
    if(MathFunctions::isEqual_f(rtControl.m_amount, 0.f))
    {
        return;
    }
    
    // Unipolar = +
    // Bipolar = ~
    
    switch(rtControl.m_src)
    {
        default:
        {
            // Source was not accounted for
            assert(false);
            break;
        }

        case ERealtimeControlSrc::FOOTSWITCH_1:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::KEY_SUSTAIN:
                {
                    // Footswitch 1 -> Key Sustain
                    const sf2cute::SFModulator fs1(sf2cute::SFMidiController::kHold, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);
                    
                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(fs1, sf2cute::SFGenerator::kSustainVolEnv, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::KEY_POLARITY_POS:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::FILTER_RES:
                {
                    // Velocity + -> 'Filter Resonance
                    const sf2cute::SFModulator velPos(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(velPos, sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::PRESSURE:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::AMP_ENV_ATTACK:
                {
                    // Pressure -> Amp Env Attack
                    const sf2cute::SFModulator pressure(sf2cute::SFGeneralController::kChannelPressure, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(pressure, sf2cute::SFGenerator::kAttackVolEnv, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::PITCH_WHEEL:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::PITCH:
                {
                    // Pitch Wheel -> Pitch
                    const sf2cute::SFModulator pitchWheel(sf2cute::SFGeneralController::kPitchWheel, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear);
                    
                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(pitchWheel, sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(std::roundf(rtControl.m_amount)),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::MOD_WHEEL:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::FILTER_FREQ:
                {
                    // Mod Wheel -> Filter Freq
                    const sf2cute::SFModulator modWheel(sf2cute::SFMidiController::kModulationDepth, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(modWheel, sf2cute::SFGenerator::kInitialFilterFc,
                        SF2Helpers::filterFreqPercentToCents(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }

                case ERealtimeControlDst::VIBRATO:
                {
                    // Mod Wheel -> Cord 3 Amount (Vibrato)
                    const sf2cute::SFModulator modWheel(sf2cute::SFMidiController::kModulationDepth, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(modWheel, sf2cute::SFGenerator::kVibLfoToPitch, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::LFO1_POLARITY_CENTER:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::FILTER_FREQ:
                {
                    // LFO 1 ~ -> Filter Frequency
                    instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToFilterFc,
                        SF2Helpers::filterFreqPercentToCents(rtControl.m_amount)));

                    break;
                }

                case ERealtimeControlDst::AMP_PAN:
                {
                    if (options.m_useConverterSpecificData)
                    {
                        // LFO 1 ~ -> Amp Pan
                        instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused1, static_cast<int16_t>(rtControl.m_amount)));
                    }
                    
                    break;
                }

                case ERealtimeControlDst::AMP_VOLUME:
                {
                    // LFO 1 ~ -> Amp Volume
                    const int16_t cB(SF2Helpers::convert_dB_to_cB(rtControl.m_amount * SF2Helpers::MIN_MAX_LFO1_TO_VOLUME / 100.f)); // Converted to [-15, 15]
                    instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToVolume, cB));
                    break;
                }
                
                case ERealtimeControlDst::PITCH:
                {
                    // LFO 1 ~ -> Pitch
                    instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToPitch, static_cast<int16_t>(rtControl.m_amount)));
                    break;
                }
            }
            
            break;
        }
        
        case ERealtimeControlSrc::FILTER_ENV_POLARITY_POS:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::FILTER_FREQ:
                {
                    // Filter Env + -> Filter Frequency
                    instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModEnvToFilterFc,
                        SF2Helpers::filterFreqPercentToCents(rtControl.m_amount)));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::PEDAL:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::AMP_VOLUME:
                {
                    // Pedal -> Amp Volume
                    const sf2cute::SFModulator pedal(sf2cute::SFMidiController::kController4, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(pedal, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::VEL_POLARITY_LESS:
        {
            const sf2cute::SFModulator velLess(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kDecrease,
                sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);
            
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::AMP_VOLUME:
                {
                    // Velocity < -> Amp Volume
                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }

                case ERealtimeControlDst::FILTER_ENV_ATTACK:
                {
                    // Velocity < -> Filter Env Attack
                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kAttackModEnv, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }

                case ERealtimeControlDst::FILTER_FREQ:
                {
                    // Velocity < -> Filter Freq
                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialFilterFc,
                        SF2Helpers::filterFreqPercentToCents(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::KEY_POLARITY_CENTER:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::FILTER_FREQ:
                {
                    // Key ~ -> Filter Freq
                    const sf2cute::SFModulator keyCenter(sf2cute::SFGeneralController::kNoteOnKeyNumber, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(keyCenter, sf2cute::SFGenerator::kInitialFilterFc,
                        SF2Helpers::filterFreqPercentToCents(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }

                case ERealtimeControlDst::AMP_PAN:
                {
                    // Velocity ~ -> Amp Pan
                    const sf2cute::SFModulator velCenter(sf2cute::SFGeneralController::kNoteOnVelocity, sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kBipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(velCenter, sf2cute::SFGenerator::kPan, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }

        case ERealtimeControlSrc::MIDI_A:
        {
            switch(rtControl.m_dst)
            {
                default:
                {
                    // Destination was not accounted for
                    assert(false);
                    break;
                }

                case ERealtimeControlDst::AMP_VOLUME:
                {
                    // MIDI A -> Amp Volume
                    const sf2cute::SFModulator midiA(sf2cute::SFMidiController::kController21,
                        sf2cute::SFControllerDirection::kIncrease,
                        sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear);

                    instrumentZone.SetModulator(sf2cute::SFModulatorItem(midiA,
                        sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(rtControl.m_amount),
                        sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
                    
                    break;
                }
            }
            
            break;
        }
    }
}

std::string SF2Writer::ConvertNameToSFName(const std::string_view& name) const
{
    std::string str(std::begin(name), std::ranges::find(name, '\0'));
    if (name.length() > SF2Helpers::SF2_MAX_NAME_LEN) { str.resize(SF2Helpers::SF2_MAX_NAME_LEN); }
    return str;
}