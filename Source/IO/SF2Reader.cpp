#include "Header/IO/SF2Reader.h"
#include "Header/Data/Soundbank.h"
#include "Header/IO/BinaryReader.h"
#include "Header/Logger.h"
#include "Header/SF2/Helpers/SF2Helpers.h"
#include "Header/BankReadOptions.h"
#include "sf2cute/generator_item.hpp"
#include "sf2cute/modulator.hpp"
#include "sf2cute/types.hpp"
#include <array>
#include <unordered_map>

// SF2 reading
#define TSF_NO_STDIO
#define TSF_IMPLEMENTATION

#include "Dependencies/TinySoundFont/tsf.h"

Soundbank SF2Reader::ProcessFile(const std::filesystem::path& file, const BankReadOptions& options)
{
    Soundbank outResult(file.filename().replace_extension("").string());
    
    BinaryReader reader;
    if(reader.readFile(file))
    {
        const auto& sf2Data(reader.GetData());
        const tsf* sf2(tsf_load_memory(sf2Data.data(), static_cast<int>(sf2Data.size())));
        assert(sf2 != nullptr);
        if (sf2 != nullptr)
        {
            const auto& numPresets(sf2->presetNum);
            if (numPresets <= 0)
            {
                Logger::LogMessage("(Bank: '%s') Preset count was <= 0", outResult.m_bankName.c_str());
                return outResult;
            }

            // This is used to keep track of the loop settings.
            std::unordered_map<uint16_t, sf2cute::SampleMode> sampleModes{};

            for (int i(0); i < numPresets; ++i)
            {
                const auto& preset(sf2->presets[i]);
                const uint16_t presetIndex(preset.preset);
                const uint16_t numVoices(static_cast<uint16_t>(preset.regionNum));

                std::vector<BankVoice> voices(numVoices);
                for (uint16_t j(0ui16); j < numVoices; ++j)
                {
                    auto& voice(voices[j]);
                    const auto& region(preset.regions[j]);
                    voice.m_filterFrequency = static_cast<int16_t>(tsf_cents2Hertz(static_cast<float>(region.initialFilterFc)));

                    const float convertedPan(std::round(SF2Helpers::relPercentToValue(region.pan)));
                    voice.m_pan = static_cast<int8_t>(options.m_flipPan ? -convertedPan : convertedPan);

                    voice.m_chorusAmount = SF2Helpers::relPercentToValue(region.chorusEffectsSend);
                    voice.m_volume = static_cast<int8_t>(region.attenuation);
                    
                    const double LFO1Freq(SF2Helpers::centsToHertz(static_cast<int16_t>(region.freqModLFO)));
                    const double LFO1Delay(static_cast<double>(region.delayModLFO));
                    voice.m_lfo1 = BankLFO(LFO1Freq, 0ui8, LFO1Delay, true);

                    // Handle converter specific data
                    if (options.m_useConverterSpecificData)
                    {
                        voice.m_lfo1 = BankLFO(LFO1Freq, static_cast<uint8_t>(region.unused3), LFO1Delay, static_cast<bool>(region.unused4));
                        voice.m_chorusWidth = SF2Helpers::relPercentToValue(region.unused5);

                        // Multiply by the sign since SF2 does not support negative attenuation
                        const auto attenuationSign(region.unused2);
                        voice.m_volume = static_cast<int8_t>(attenuationSign != 0
                            ? region.attenuation * static_cast<float>(attenuationSign)
                            : region.attenuation);
                    }
                    
                    voice.m_fineTune = static_cast<double>(region.tune);
                    voice.m_coarseTune = static_cast<int8_t>(region.transpose);
                    voice.m_filterQ = static_cast<float>(region.initialFilterQ) / 10.f;
                    voice.m_keyZone = BankNoteRange(region.lokey, region.hikey);
                    voice.m_velocityZone = BankNoteRange(region.lovel, region.hivel);
                    voice.m_originalKey = static_cast<uint8_t>(region.pitch_keycenter);
                    
                    voice.m_sampleIndex = region.sampleIndex;
                    sampleModes.insert_or_assign(voice.m_sampleIndex, static_cast<sf2cute::SampleMode>(region.loop_mode));

                    const auto& ampEnv(region.ampenv);
                    const float keyDelay(MathFunctions::round_f_places(ampEnv.delay, 3u));
                    voice.m_ampEnv = ADSR_Envelope(static_cast<double>(ampEnv.attack), static_cast<double>(ampEnv.decay), static_cast<double>(ampEnv.hold),
                        ampEnv.sustain * 100.f, static_cast<double>(ampEnv.release), static_cast<double>(keyDelay));

                    const auto& filterEnv(region.modenv);
                    const float filterDelay(MathFunctions::round_f_places(filterEnv.delay, 3u));
                    voice.m_filterEnv = ADSR_Envelope(static_cast<double>(filterEnv.attack), static_cast<double>(filterEnv.decay), static_cast<double>(filterEnv.hold),
                        filterEnv.sustain * 100.f, static_cast<double>(filterEnv.release), static_cast<double>(filterDelay));

                    // Apply the defaults if completely zeroed.
                    // TODO: Find a better way to check if the SF2 has unmodified filter settings
                    if(voice.m_filterEnv.IsZeroed())
                    {
                        voice.m_filterEnv = options.m_filterDefaults;
                    }

                    /*
                     * Realtime controls:
                     */

                    if(region.modLfoToPitch != 0)
                    {
                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::LFO1_POLARITY_CENTER, ERealtimeControlDst::PITCH, static_cast<int8_t>(region.modLfoToPitch));
                    }

                    if(region.modEnvToFilterFc != 0)
                    {
                        const int16_t modEnvToFilterFreq(static_cast<int16_t>(region.modEnvToFilterFc));
                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::FILTER_ENV_POLARITY_POS, ERealtimeControlDst::FILTER_FREQ,
                            SF2Helpers::centsToFilterFreqPercent(modEnvToFilterFreq));
                    }

                    if(region.modLfoToVolume != 0)
                    {
                        const int16_t modLfoToVolume(static_cast<int16_t>(region.modLfoToVolume));
                        const float dB(SF2Helpers::convert_cB_to_dB(modLfoToVolume));
                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::LFO1_POLARITY_CENTER, ERealtimeControlDst::AMP_VOLUME,
                        dB * 100.f / SF2Helpers::MIN_MAX_LFO1_TO_VOLUME);
                    }

                    if(region.modLfoToFilterFc != 0)
                    {
                        const int16_t modLfoToFilterFc(static_cast<int16_t>(region.modLfoToFilterFc));
                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::LFO1_POLARITY_CENTER, ERealtimeControlDst::FILTER_FREQ,
                        SF2Helpers::centsToFilterFreqPercent(modLfoToFilterFc));
                    }

                    if (options.m_useConverterSpecificData)
                    {
                        if(region.unused1 != 0)
                        {
                            const int8_t LFO1ToAmpPan(static_cast<int8_t>(region.unused1));
                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::LFO1_POLARITY_CENTER, ERealtimeControlDst::AMP_PAN, LFO1ToAmpPan);
                        }
                    }
                    
                    for (const auto& mod : region.modulators)
                    {
                        const sf2cute::SFModulator srcOper(mod.modSrcOper);
                        const float modAmountF(static_cast<float>(mod.modAmount));
                        const auto destOper(static_cast<sf2cute::SFGenerator>(mod.modDestOper));

                        // Skip 0 amounts
                        if(mod.modAmount == 0ui16) { continue; }
                        
                        if (srcOper.controller_palette() == sf2cute::SFControllerPalette::kGeneralController)
                        {
                            if (srcOper.general_controller() == sf2cute::SFGeneralController::kPitchWheel)
                            {
                                if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                {
                                    if (destOper == sf2cute::SFGenerator::kFineTune)
                                    {
                                        if (srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
                                        {
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::PITCH_WHEEL,
                                                ERealtimeControlDst::PITCH, modAmountF);
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
                                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::VEL_POLARITY_POS,
                                            ERealtimeControlDst::FILTER_RES, modAmountF);
                                    }
                                    else if (destOper == sf2cute::SFGenerator::kPan)
                                    {
                                        if (srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
                                        {
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::VEL_POLARITY_CENTER,
                                                ERealtimeControlDst::AMP_PAN, modAmountF);
                                        }
                                    }
                                }
                                else if (srcOper.direction() == sf2cute::SFControllerDirection::kDecrease)
                                {
                                    if (destOper == sf2cute::SFGenerator::kInitialAttenuation)
                                    {
                                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::VEL_POLARITY_LESS,
                                            ERealtimeControlDst::AMP_VOLUME, modAmountF);
                                    }
                                    else if (destOper == sf2cute::SFGenerator::kAttackModEnv)
                                    {
                                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::VEL_POLARITY_LESS,
                                            ERealtimeControlDst::FILTER_ENV_ATTACK, modAmountF);
                                    }
                                    else if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
                                    {
                                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::VEL_POLARITY_LESS,
                                            ERealtimeControlDst::FILTER_FREQ,
                                            SF2Helpers::centsToFilterFreqPercent(mod.modAmount));
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
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::KEY_POLARITY_CENTER,
                                                ERealtimeControlDst::FILTER_FREQ,
                                                SF2Helpers::centsToFilterFreqPercent(mod.modAmount));
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
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::PRESSURE,
                                                ERealtimeControlDst::AMP_ENV_ATTACK, modAmountF);
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
                                        voice.ReplaceOrAddRTControl(ERealtimeControlSrc::MIDI_A,
                                            ERealtimeControlDst::AMP_VOLUME, modAmountF);
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
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::MOD_WHEEL,
                                                ERealtimeControlDst::FILTER_FREQ,
                                                SF2Helpers::centsToFilterFreqPercent(mod.modAmount));
                                        }
                                    }
                                    else if (destOper == sf2cute::SFGenerator::kVibLfoToPitch)
                                    {
                                        if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                        {
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::MOD_WHEEL,
                                                ERealtimeControlDst::VIBRATO, modAmountF);
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
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::PEDAL,
                                                ERealtimeControlDst::AMP_VOLUME, modAmountF);
                                        }
                                    }
                                }
                            }
                            else if(srcOper.midi_controller() == sf2cute::SFMidiController::kHold)
                            {
                                if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
                                {
                                    if (destOper == sf2cute::SFGenerator::kSustainVolEnv)
                                    {
                                        if (srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
                                        {
                                            voice.ReplaceOrAddRTControl(ERealtimeControlSrc::FOOTSWITCH_1,
                                                ERealtimeControlDst::KEY_SUSTAIN, modAmountF);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                outResult.m_presets.emplace_back(presetIndex, std::string(preset.presetName.data()), std::move(voices));
            }

            uint16_t sampleIndex(0ui16);
            for (const auto& shdr : sf2->shdrs)
            {
                if(shdr.start == 0u && shdr.end == 0u)
                {
                    Logger::LogMessage("(Bank: '%s', Sample: '%s'): Skipped sample with a start of 0 and end of 0.", outResult.m_bankName.c_str(), shdr.sampleName.data());
                    continue;
                }

                const auto sampleType(static_cast<sf2cute::SFSampleLink>(shdr.sampleType));
                                        
                if(sampleType != sf2cute::SFSampleLink::kMonoSample)
                {
                    Logger::LogMessage("(Bank: '%s', Sample: '%s'): Sample type is unsupported!", outResult.m_bankName.c_str(), shdr.sampleName.data());
                    continue;
                }

                const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
                if (sampleSize > 0u)
                {
                    const uint32_t loopStart(shdr.startLoop - shdr.start);
                    const uint32_t loopEnd(shdr.endLoop - shdr.start);

                    bool isLooping(false);
                    bool isLoopReleasing(false);
                    
                    const auto& sampleFind(sampleModes.find(sampleIndex));
                    assert(sampleFind != sampleModes.end());
                    if(sampleFind != sampleModes.end())
                    {
                        const auto loopMode(sampleFind->second);
                        isLooping = loopMode == sf2cute::SampleMode::kLoopContinuously ||
                            loopMode == sf2cute::SampleMode::kLoopEndsByKeyDepression;

                        if(isLooping)
                        {
                            const bool isValidLoop = loopStart < loopEnd && loopEnd > loopStart;
                            assert(isValidLoop);
                            if(isValidLoop)
                            {
                                isLoopReleasing = loopMode == sf2cute::SampleMode::kLoopEndsByKeyDepression;
                            }
                            else
                            {
                                isLooping = false;
                            }
                        }
                    }
                    
                    if(sampleType == sf2cute::SFSampleLink::kMonoSample)
                    {
                        std::vector sampleData(&sf2->samplesAsShort[shdr.start], &sf2->samplesAsShort[shdr.end]);
                        outResult.m_samples.emplace_back(sampleIndex, std::string(shdr.sampleName.data()),
                            std::move(sampleData), shdr.sampleRate, 1u, isLooping, isLoopReleasing, loopStart, loopEnd);
                    }
                    else if(sampleType == sf2cute::SFSampleLink::kLeftSample)
                    {
                        // TODO: left sample
                    }
                    else if(sampleType == sf2cute::SFSampleLink::kRightSample)
                    {
                        // TODO: right sample
                    }
                    else
                    {
                        Logger::LogMessage("(Bank: '%s', Sample: '%s'): Sample type is unsupported!", outResult.m_bankName.c_str(), shdr.sampleName.data());
                        continue;
                    }
                }

                ++sampleIndex;
            }
        }
    }
    
    return outResult;
}
