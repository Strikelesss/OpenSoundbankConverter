#include "Header/BankConverter.h"
#include "Header/BinaryWriter.h"
#include "Header/E4Preset.h"
#include "Header/E4Sample.h"
#include "Header/VoiceDefinitions.h"
#include <filesystem>
#include <fstream>
#include <iostream>

#define TSF_IMPLEMENTATION
#include "Dependencies/TinySoundFont/tsf.h"

#include <sf2cute.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include <tchar.h>

//
// These are NOT correct but will work until we can determine how each sound in the SF2
// TODO: correct these values

int16_t SF2Converter::filterFreqPercentToCents(const float filterFreq)
{
	if(filterFreq > MAX_FILTER_FREQ_HZ_CORDS || filterFreq < -MAX_FILTER_FREQ_HZ_CORDS) { OutputDebugStringA("Invalid filter frequency"); return 0i16; }
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
	if(e4b.GetSamples().empty() || e4b.GetPresets().empty() || bankName.empty()) { return false; }

	const auto convertedName(ConvertNameToSFName(bankName));

	sf2cute::SoundFont sf2;
	sf2.set_bank_name(convertedName);
	sf2.set_sound_engine("EMU8000");

	for(const auto& e4Sample : e4b.GetSamples())
	{
		sf2.NewSample(e4Sample.GetName(), e4Sample.GetData(), e4Sample.GetLoopStart(), e4Sample.GetLoopEnd(), e4Sample.GetSampleRate(), 0, 0ui8);
	}

	uint32_t presetIndex(0u);
	for(const auto& preset : e4b.GetPresets())
	{
		std::vector<sf2cute::SFPresetZone> presetZones;
		std::vector<sf2cute::SFInstrumentZone> instrumentZones;
		for(const auto& voice : preset.GetVoices())
		{
			const auto sampleIndex(e4b.GetSampleIndexMapping().at(voice.GetSampleIndex()));
			const auto& e4Sample(e4b.GetSamples()[sampleIndex]);

			uint16_t sampleMode(0ui16);
			if (e4Sample.IsLooping()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopContinuously); }
			if (e4Sample.IsReleasing()) { sampleMode |= 2; }

			const auto& zoneRange(voice.GetZoneRange());
			const auto& velRange(voice.GetVelocityRange());

			const auto voiceVolBefore(voice.GetVolume());
			const auto voiceVolume(std::clamp(static_cast<int16_t>(std::abs(voiceVolBefore) * 10i16), 0i16, 144i16)); // Using abs on volume since SF2 does not support negative attenuation

			sf2cute::SFInstrumentZone instrumentZone(sf2.samples()[sampleIndex], std::vector{
				sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kKeyRange, sf2cute::RangesType(zoneRange.first, zoneRange.second)),
				sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kVelRange, sf2cute::RangesType(velRange.first, velRange.second))
			}, std::vector<sf2cute::SFModulatorItem>{});

			if(voiceVolume != 0i16)
			{
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialAttenuation, voiceVolume));
			}

			const auto originalKey(voice.GetOriginalKey());
			if(originalKey != 53ui8)
			{
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kOverridingRootKey, originalKey));
			}

			if(sampleMode != 0ui16)
			{
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSampleModes, static_cast<int16_t>(sampleMode)));
			}

			// Envelope
			// TODO: plot points from E4B onto an ADSR envelope and grab the time, since the time is inaccurate below (a binary value of 126 could be 2.1 sec, when it normally is say 80 sec)

			/*
			 * Amp Env
			 * NOTE: for Delay we use the one provided in 'Key' but in reality it should be Attack1Sec
			 * Generally Attack1Sec is only used for Filter Env, so that is used there
			 */

			const auto& ampEnv(voice.GetAmpEnv());
			const auto ampDelaySec(SF2Converter::secToTimecent(voice.GetKeyDelay()));
			if (ampDelaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayVolEnv, ampDelaySec)); }

			const auto ampAttackSec(SF2Converter::secToTimecent(ampEnv.GetAttack2Sec()));
			if (ampAttackSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackVolEnv, ampAttackSec)); }

			const auto ampHoldSec(SF2Converter::secToTimecent(ampEnv.GetDecay1Sec()));
			if (ampHoldSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kHoldVolEnv, ampHoldSec)); }

			// Sustain Level is expressed in dB for vol env, and is also opposite because of SF2
			const auto ampSustainLevel(ampEnv.GetDecay2Level());
			if(ampSustainLevel < 100.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainVolEnv, static_cast<int16_t>((-(ampSustainLevel / 100.f * 144.f) + 144.f) * 10.f))); }

			const auto ampDecaySec(SF2Converter::secToTimecent(ampEnv.GetDecay2Sec()));
			if(ampSustainLevel < 100.f && ampDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayVolEnv, ampDecaySec)); }

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
			if(filterSustainLevel < 100.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainModEnv, static_cast<int16_t>((-filterSustainLevel + 100.f) * 10.f))); }

			const auto filterDecaySec(SF2Converter::secToTimecent(filterEnv.GetDecay2Sec()));
			if(filterSustainLevel < 100.f && filterDecaySec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayModEnv, filterDecaySec)); }

			const auto filterReleaseSec(SF2Converter::secToTimecent(filterEnv.GetRelease1Sec()));
			if (filterReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseModEnv, filterReleaseSec)); }

			// Filters

			const auto filterFreqCents(VoiceDefinitions::hertzToCents(voice.GetFilterFrequency()));
			if (filterFreqCents >= 1500i16 && filterFreqCents < 13500i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, filterFreqCents)); }

			const auto filterQ(voice.GetFilterQ());
			if (filterQ > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(filterQ * 10.f))); }

			// LFO

			const auto lfo1Freq(VoiceDefinitions::hertzToCents(voice.GetLFO1().GetRate()));
			if (lfo1Freq != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFreqModLFO, lfo1Freq)); }

			const auto lfo1Delay(SF2Converter::secToTimecent(voice.GetLFO1().GetDelay()));
			if (lfo1Delay != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDelayModLFO, lfo1Delay)); }

			if(options.m_useConverterSpecificData)
			{
				const auto lfo1Shape(voice.GetLFO1().GetShape());
				if (lfo1Shape != 0ui8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused3, lfo1Shape)); }

				const auto lfo1KeySync(voice.GetLFO1().IsKeySync());
				if(lfo1KeySync) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused4, true)); }
			}

			// Cords
			// Unipolar = +
			// Bipolar = ~

			float cordAmtPercent1(0.f);
			if(voice.GetAmountFromCord(LFO1_POLARITY_CENTER, AMP_VOLUME, cordAmtPercent1))
			{
				// LFO 1 ~ -> Amp Volume
				const auto cB(VoiceDefinitions::convert_dB_to_cB(cordAmtPercent1 * SF2Converter::MIN_MAX_LFO1_TO_VOLUME / 100.f)); // Converted to -15, 15
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToVolume, cB));
			}

			float cordAmtPercent2(0.f);
			if(voice.GetAmountFromCord(LFO1_POLARITY_CENTER, PITCH, cordAmtPercent2))
			{
				// LFO 1 ~ -> Pitch
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModLfoToPitch, static_cast<int16_t>(cordAmtPercent2)));
			}

			float cordAmtPercent3(0.f);
			if(voice.GetAmountFromCord(LFO1_POLARITY_CENTER, FILTER_FREQ, cordAmtPercent3))
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
			if(voice.GetAmountFromCord(FILTER_ENV_POLARITY_POS, FILTER_FREQ, cordAmtPercent5))
			{
				// Filter Env + -> Filter Frequency
				instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kModEnvToFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent5)));
			}

			float cordAmtPercent6(0.f);
			if(voice.GetAmountFromCord(PITCH_WHEEL, PITCH, cordAmtPercent6))
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
				const auto midiA(sf2cute::SFModulator(sf2cute::SFMidiController::kController2, sf2cute::SFControllerDirection::kIncrease,
					sf2cute::SFControllerPolarity::kUnipolar, sf2cute::SFControllerType::kLinear));

				instrumentZone.SetModulator(sf2cute::SFModulatorItem(midiA, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(cordAmtPercent7),
					sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
			}

			float cordAmtPercent8(0.f);
			if (voice.GetAmountFromCord(VELOCITY_POLARITY_POS, FILTER_RES, cordAmtPercent8))
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
			if (voice.GetAmountFromCord(VELOCITY_POLARITY_LESS, AMP_VOLUME, cordAmtPercent9))
			{
				// Velocity < -> Amp Volume
				instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(cordAmtPercent9),
					sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
			}

			float cordAmtPercent10(0.f);
			if (voice.GetAmountFromCord(VELOCITY_POLARITY_LESS, FILTER_ENV_ATTACK, cordAmtPercent10))
			{
				// Velocity < -> Filter Env Attack
				instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kAttackModEnv, static_cast<int16_t>(cordAmtPercent10),
					sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
			}

			float cordAmtPercent11(0.f);
			if (voice.GetAmountFromCord(VELOCITY_POLARITY_LESS, FILTER_FREQ, cordAmtPercent11))
			{
				// Velocity < -> Filter Freq
				instrumentZone.SetModulator(sf2cute::SFModulatorItem(velLess, sf2cute::SFGenerator::kInitialFilterFc, SF2Converter::filterFreqPercentToCents(cordAmtPercent11), 
					sf2cute::SFModulator(), sf2cute::SFTransform::kAbsoluteValue));
			}

			float cordAmtPercent12(0.f);
			if (voice.GetAmountFromCord(VELOCITY_POLARITY_CENTER, AMP_PAN, cordAmtPercent12))
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

			// Amplifier / Oscillator

			const auto pan(options.m_flipPan ? -voice.GetPan() : voice.GetPan());
			if (pan != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, static_cast<int16_t>(pan * 10i16))); }

			const auto fineTune(voice.GetFineTune());
			if (fineTune != 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(std::round(fineTune)))); }

			const auto coarseTune(voice.GetCoarseTune());
			if (coarseTune != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kCoarseTune, coarseTune)); }

			const auto chorusSend(voice.GetChorusAmount());
			if (chorusSend > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend, static_cast<int16_t>(chorusSend * 10.f))); }

			// Other

			if(options.m_useConverterSpecificData)
			{
				const auto chorusWidth(voice.GetChorusWidth());
				if (chorusWidth > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused5, static_cast<int16_t>(chorusWidth))); }

				const auto attenuationSign(voiceVolBefore > 0 ? 1 : voiceVolBefore < 0 ? -1 : 0);
				if(attenuationSign != 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kUnused2, static_cast<int16_t>(attenuationSign))); }
			}

			instrumentZones.emplace_back(instrumentZone);
		}

		const auto& presetName(preset.GetName());
		presetZones.emplace_back(sf2.NewInstrument(presetName, instrumentZones));
		const std::shared_ptr sfPreset(sf2.NewPreset(presetName, presetIndex, 0, presetZones));
		++presetIndex;
	}

	try 
	{
		if (!options.m_ignoreFileNameSetting)
		{
			auto sf2Path(std::filesystem::path(convertedName).wstring());
			sf2Path.resize(MAX_PATH);

			OPENFILENAME ofn{};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = nullptr;
			ofn.lpstrFilter = _T(".sf2");
			ofn.lpstrFile = sf2Path.data();
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER;
			ofn.lpstrDefExt = _T("sf2");

			if (GetSaveFileName(&ofn))
			{
				std::ofstream ofs(ofn.lpstrFile, std::ios::binary);
				sf2.Write(ofs);
				return true;
			}
		}
		else
		{
			auto path(options.m_ignoreFileNameSettingSaveFolder);
			if(!path.empty() && exists(path))
			{
				auto sf2Path(path.append(std::filesystem::path(convertedName + ".sf2").wstring()));
				std::ofstream ofs(sf2Path, std::ios::binary);
				sf2.Write(ofs);
				return true;
			}
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

bool BankConverter::ConvertSF2ToE4B(const std::filesystem::path& bank, const std::string_view& bankName, const ConverterOptions& options) const
{
	if(exists(bank) && !bankName.empty())
	{
		std::string sf2PathTemp(bankName);
		sf2PathTemp.resize(MAX_PATH);

		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = ".E4B";
		ofn.lpstrFile = sf2PathTemp.data();
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER;
		ofn.lpstrDefExt = "E4B";

		if (GetSaveFileNameA(&ofn))
		{
			const std::filesystem::path sf2Path(sf2PathTemp);
			BinaryWriter writer(sf2Path);

			const tsf* sf2(tsf_load_filename(bank.string().c_str()));
			if(sf2 != nullptr)
			{
				const bool canUseConverterSpecificData(options.m_useConverterSpecificData && !options.m_isChickenTranslatorFile);

				if(writer.writeType(E4BVariables::EOS_FORM_TAG.data(), E4BVariables::EOS_FORM_TAG.length()))
				{
					const auto numPresets(sf2->presetNum);
					if(numPresets <= 0) { return false; }

					std::unordered_map<size_t, size_t> presetChunkLocations{}, sampleChunkLocations{};

					// E4B0 + TOC1 + LENGTH
					auto totalSize(static_cast<uint32_t>(E4BVariables::EOS_E4_FORMAT_TAG.length() + E4BVariables::EOS_TOC_TAG.length() + sizeof(uint32_t)));

					totalSize += numPresets * (E4BVariables::EOS_CHUNK_TOTAL_LEN + E4BVariables::EOS_CHUNK_NAME_OFFSET + TOTAL_PRESET_DATA_SIZE);

					uint32_t tocSize(numPresets * E4BVariables::EOS_CHUNK_TOTAL_LEN);

					const auto shdrs(sf2->shdrs);
					for (const auto& shdr : shdrs)
					{
						const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
						if (sampleSize > 0u)
						{
							totalSize += E4BVariables::EOS_CHUNK_TOTAL_LEN + E4BVariables::EOS_CHUNK_NAME_OFFSET + TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize;
							tocSize += E4BVariables::EOS_CHUNK_TOTAL_LEN;
						}
					} 

					for(int i(0); i < numPresets; ++i)
					{
						const auto preset(sf2->presets[i]);
						totalSize += preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE);
					}

					totalSize += static_cast<uint32_t>(E4BVariables::EOS_EMSt_TAG.length());

					// TODO: ensure this is the correct size
					totalSize += TOTAL_EMST_DATA_SIZE;

					// Byteswap since E4B requires it
					totalSize = _byteswap_ulong(totalSize);
					tocSize = _byteswap_ulong(tocSize);

					if(writer.writeType(&totalSize))
					{
						if(writer.writeType(E4BVariables::EOS_E4_FORMAT_TAG.data(), E4BVariables::EOS_E4_FORMAT_TAG.length()))
						{
							if(writer.writeType(E4BVariables::EOS_TOC_TAG.data(), E4BVariables::EOS_TOC_TAG.length()))
							{
								if(writer.writeType(&tocSize))
								{
									constexpr uint16_t redundant16(0ui16);

									/*
									 * Chunks
									 */

									for(int i(0); i < numPresets; ++i)
									{
										if(writer.writeType(E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()))
										{
											const auto preset(sf2->presets[i]);
											const uint32_t presetChunkLen(_byteswap_ulong(TOTAL_PRESET_DATA_SIZE + preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
											if(writer.writeType(&presetChunkLen))
											{
												presetChunkLocations[i] = writer.GetWritePos();

												constexpr uint32_t presetChunkLoc(0u);
												if(writer.writeType(&presetChunkLoc))
												{
													const auto presetNum(static_cast<uint16_t>(i));
													if(writer.writeType(&presetNum) && writer.writeType(ConvertNameToEmuName(preset.presetName).c_str(), E4BVariables::EOS_E4_MAX_NAME_LEN)
														&& writer.writeType(&redundant16))
													{
														OutputDebugStringA("wrote preset \n");
													}
												}
											}
										}
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

													constexpr uint32_t presetChunkLoc(0u);
													if (writer.writeType(&presetChunkLoc))
													{
														const auto sampleIndexBS(_byteswap_ushort(sampleIndex));
														if(writer.writeType(&sampleIndexBS) && writer.writeType(ConvertNameToEmuName(shdr.sampleName).c_str(), 
															E4BVariables::EOS_E4_MAX_NAME_LEN) && writer.writeType(&redundant16))
														{
															OutputDebugStringA("wrote sample \n");
														}
													}
												}
											}

											++sampleIndex;
										}
									}

									/*
									 * Data
									 */

									std::unordered_map<uint8_t, uint8_t> sampleModes{};

									for(int i(0); i < numPresets; ++i)
									{
										const auto currentPos(_byteswap_ulong(static_cast<uint32_t>(writer.GetWritePos())));
										if (writer.writeTypeAtLocation(&currentPos, presetChunkLocations[i]) && writer.writeType(E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()))
										{
											const auto preset(sf2->presets[i]);
											uint32_t presetChunkLen(_byteswap_ulong(sizeof(uint16_t) + TOTAL_PRESET_DATA_SIZE + preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
											if (writer.writeType(&presetChunkLen))
											{
												const auto presetNum(_byteswap_ushort(static_cast<uint16_t>(i)));
												if (writer.writeType(&presetNum) && writer.writeType(ConvertNameToEmuName(preset.presetName).c_str(), E4BVariables::EOS_E4_MAX_NAME_LEN))
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
																constexpr std::array<uint8_t, 8> redundantPresetData2{ 'R', '#', '\0', '~', static_cast<uint8_t>(255),
																	static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255) };

																if (writer.writeType(redundantPresetData2.data(), sizeof(char) * redundantPresetData2.size()))
																{
																	constexpr std::array<int8_t, 24> redundantPresetData3{};
																	if (writer.writeType(redundantPresetData3.data(), sizeof(int8_t) * redundantPresetData3.size()))
																	{
																		for (uint16_t j(0ui16); j < numVoices; ++j)
																		{
																			const auto& region(preset.regions[j]);
																			const auto filterFreq(static_cast<int16_t>(tsf_cents2Hertz(static_cast<float>(region.initialFilterFc))));
																			auto convertedPan(region.pan / 10.f);

																			// Chicken Translator clamps -50 to 50, while EOS uses -64 to 63
																			if(options.m_isChickenTranslatorFile)
																			{
																				constexpr auto PAN_CONVERT_INV(1.f / 0.775f);
																				if(convertedPan > 0.f || convertedPan < 0.f) { convertedPan *= PAN_CONVERT_INV; }
																			}

																			convertedPan = std::round(convertedPan);

																			const auto pan(static_cast<int8_t>(options.m_flipPan ? -convertedPan : convertedPan));

																			int8_t volume(0i8);

																			if(canUseConverterSpecificData)
																			{
																				// Multiply by the sign since SF2 does not support negative attenuation
																				const auto attenuationSign(region.unused2);
																				volume = static_cast<int8_t>(attenuationSign != 0 ? region.attenuation * 
																					static_cast<float>(attenuationSign) : region.attenuation);
																			}

																			const auto fineTune(static_cast<double>(region.tune));
																			const auto coarseTune(static_cast<int8_t>(region.transpose));
																			const auto filterQ(std::roundf(static_cast<float>(region.initialFilterQ) / 10.f));

																			const auto& ampEnv(region.ampenv);

																			// Round up 3 decimal places
																			const auto keyDelay(std::ceil(static_cast<double>(ampEnv.delay) * 1000.) / 1000.);

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

																			float chorusAmount(region.chorusEffectsSend / 10.f);
																			auto chorusWidth(static_cast<float>(region.unused5));
																			const auto LFO1Freq(VoiceDefinitions::centsToHertz(static_cast<int16_t>(region.freqModLFO)));
																			const auto LFO1Delay(static_cast<double>(region.delayModLFO));

																			E4LFO lfo1;
																			if(canUseConverterSpecificData)
																			{
																				lfo1 = E4LFO(LFO1Freq, static_cast<uint8_t>(region.unused3), LFO1Delay, region.unused4);
																				chorusWidth = static_cast<float>(region.unused5);
																			}
																			else
																			{
																				lfo1 = E4LFO(LFO1Freq, 0ui8, LFO1Delay, false);
																			}

																			E4Voice voice(chorusWidth, chorusAmount, filterFreq, coarseTune, pan, volume, fineTune, keyDelay, filterQ, { region.lokey, region.hikey },
																				{ region.lovel, region.hivel }, E4Envelope(ampAttack, ampDecay, ampHold, ampRelease, 0., ampSustain), 
																				E4Envelope(filterAttack, filterDecay, filterHold, filterRelease, filterDelay, filterSustain), lfo1);

																			/*
																			 * Cords
																			 */

																			voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, PITCH, static_cast<int8_t>(region.modLfoToPitch));

																			const auto modEnvToFilterFreq(static_cast<int16_t>(region.modEnvToFilterFc));
																			voice.ReplaceOrAddCord(FILTER_ENV_POLARITY_POS, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
																				SF2Converter::centsToFilterFreqPercent(modEnvToFilterFreq)));

																			const auto modLfoToVolume(static_cast<int16_t>(region.modLfoToVolume));
																			if(modLfoToVolume != 0i16)
																			{
																				const auto dB(VoiceDefinitions::convert_cB_to_dB(modLfoToVolume));
																				voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(dB * 100.f / SF2Converter::MIN_MAX_LFO1_TO_VOLUME));
																			}

																			const auto modLfoToFilterFc(static_cast<int16_t>(region.modLfoToFilterFc));
																			if(modLfoToFilterFc != 0i16) { voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, FILTER_FREQ, SF2Converter::centsToFilterFreqPercent(modLfoToFilterFc)); }

																			if(canUseConverterSpecificData)
																			{
																				const auto LFO1ToAmpPan(static_cast<int8_t>(region.unused1));
																				if(LFO1ToAmpPan != 0i8) { voice.ReplaceOrAddCord(LFO1_POLARITY_CENTER, AMP_PAN, VoiceDefinitions::ConvertPercentToByteF(LFO1ToAmpPan)); }
																			}

																			for(const auto& mod : region.modulators)
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
																									if(srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
																									{
																										voice.ReplaceOrAddCord(PITCH_WHEEL, PITCH, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
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
																									voice.ReplaceOrAddCord(VELOCITY_POLARITY_POS, FILTER_RES, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																								}
																								else if(destOper == sf2cute::SFGenerator::kPan)
																								{
																									if(srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
																									{
																										voice.ReplaceOrAddCord(VELOCITY_POLARITY_CENTER, AMP_PAN, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																									}
																								}
																							}
																							else if (srcOper.direction() == sf2cute::SFControllerDirection::kDecrease)
																							{
																								if (destOper == sf2cute::SFGenerator::kInitialAttenuation)
																								{
																									voice.ReplaceOrAddCord(VELOCITY_POLARITY_LESS, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																								}
																								else if (destOper == sf2cute::SFGenerator::kAttackModEnv)
																								{
																									voice.ReplaceOrAddCord(VELOCITY_POLARITY_LESS, FILTER_ENV_ATTACK, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																								}
																								else if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
																								{
																									voice.ReplaceOrAddCord(VELOCITY_POLARITY_LESS, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
																										SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
																								}
																							}
																						}
																						else if(srcOper.general_controller() == sf2cute::SFGeneralController::kNoteOnKeyNumber)
																						{
																							if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
																							{
																								if (destOper == sf2cute::SFGenerator::kInitialFilterFc)
																								{
																									if(srcOper.polarity() == sf2cute::SFControllerPolarity::kBipolar)
																									{
																										voice.ReplaceOrAddCord(KEY_POLARITY_CENTER, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
																											SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
																									}
																								}
																							}
																						}
																						else if(srcOper.general_controller() == sf2cute::SFGeneralController::kChannelPressure)
																						{
																							if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
																							{
																								if (destOper == sf2cute::SFGenerator::kAttackVolEnv)
																								{
																									if(srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
																									{
																										voice.ReplaceOrAddCord(PRESSURE, AMP_ENV_ATTACK, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																									}
																								}
																							}
																						}
																					}
																					else if (srcOper.controller_palette() == sf2cute::SFControllerPalette::kMidiController)
																					{
																						if (srcOper.midi_controller() == sf2cute::SFMidiController::kController2)
																						{
																							if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
																							{
																								if(destOper == sf2cute::SFGenerator::kInitialAttenuation)
																								{
																									voice.ReplaceOrAddCord(MIDI_A, AMP_VOLUME, VoiceDefinitions::ConvertPercentToByteF(modAmountF));
																								}
																							}
																						}
																						else if (srcOper.midi_controller() == sf2cute::SFMidiController::kModulationDepth)
																						{
																							if (srcOper.direction() == sf2cute::SFControllerDirection::kIncrease)
																							{
																								if(destOper == sf2cute::SFGenerator::kInitialFilterFc)
																								{
																									if(srcOper.polarity() == sf2cute::SFControllerPolarity::kUnipolar)
																									{
																										voice.ReplaceOrAddCord(MOD_WHEEL, FILTER_FREQ, VoiceDefinitions::ConvertPercentToByteF(
																											SF2Converter::centsToFilterFreqPercent(mod.modAmount)));
																									}
																								}
																							}
																						}
																					}
																				}
																			}

																			const auto originalKey(static_cast<uint8_t>(region.pitch_keycenter));
																			E4VoiceEndData voiceEnd(region.sampleIndex, originalKey);

																			if(!sampleModes.contains(region.sampleIndex)) { sampleModes[region.sampleIndex] = static_cast<uint8_t>(region.loop_mode); }
																			else
																			{
																				// Account for the weird loop mode issue where it exists for a region and not for another (EOS only supports looping per sample, not per voice..)
																				if(options.m_isChickenTranslatorFile)
																				{
																					if(sampleModes[region.sampleIndex] == TSF_LOOPMODE_NONE && region.loop_mode != TSF_LOOPMODE_NONE)
																					{
																						sampleModes[region.sampleIndex] = static_cast<uint8_t>(region.loop_mode);
																					}
																				}
																			}

																			if (voice.write(writer) && voiceEnd.write(writer))
																			{
																				OutputDebugStringA("wrote voice data \n");
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}

									sampleIndex = 0ui16;
									for (const auto& shdr : shdrs)
									{
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
													if (writer.writeType(&sampleIndexBS) && writer.writeType(ConvertNameToEmuName(shdr.sampleName).c_str(), E4BVariables::EOS_E4_MAX_NAME_LEN))
													{
														const uint32_t lastSampleLeft(sampleSize - 2ull + TOTAL_SAMPLE_DATA_READ_SIZE);

														uint32_t lastSampleRight(0u);
														if(lastSampleLeft > TOTAL_SAMPLE_DATA_READ_SIZE)
														{
															lastSampleRight = lastSampleLeft - TOTAL_SAMPLE_DATA_READ_SIZE;
														}

														uint32_t loopStart(0u);
														if(shdr.startLoop > shdr.start)
														{
															loopStart = (shdr.startLoop - shdr.start) * 2u + static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE);
														}

														uint32_t loopEnd(0u);
														if(shdr.endLoop > shdr.start)
														{
															loopEnd = (shdr.endLoop - shdr.start) * 2u + static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE);
														}

														uint32_t loopStart2(0u);
														if(loopStart > TOTAL_SAMPLE_DATA_READ_SIZE)
														{
															loopStart2 = loopStart - TOTAL_SAMPLE_DATA_READ_SIZE;
														}

														uint32_t loopEnd2(0u);
														if(loopEnd > TOTAL_SAMPLE_DATA_READ_SIZE)
														{
															loopEnd2 = loopEnd - TOTAL_SAMPLE_DATA_READ_SIZE;
														}

														// TODO: account for stereo in param 3 and '0u' in param 1

														const std::array params1{ 0u, static_cast<uint32_t>(TOTAL_SAMPLE_DATA_READ_SIZE), 0u,
															lastSampleLeft, lastSampleRight, loopStart, loopStart2, loopEnd, loopEnd2
														};

														if (writer.writeType(params1.data(), sizeof(uint32_t) * params1.size()))
														{
															if (writer.writeType(&shdr.sampleRate))
															{
																constexpr auto MONO_SAMPLE(0x00300001);

																// todo: support for stereo

																uint32_t format(MONO_SAMPLE);

																const auto& mode(sampleModes[static_cast<uint8_t>(sampleIndex)]);
																if(mode == TSF_LOOPMODE_CONTINUOUS) { format |= E4SampleVariables::SAMPLE_LOOP_FLAG; }
																else if(mode == TSF_LOOPMODE_SUSTAIN) { format |= E4SampleVariables::SAMPLE_RELEASE_FLAG; }
																else if(mode == TSF_LOOPMODE_CONTINUOUS_SUSTAIN) { format |= E4SampleVariables::SAMPLE_RELEASE_FLAG | E4SampleVariables::SAMPLE_LOOP_FLAG; }
																
																if (writer.writeType(&format))
																{
																	// Generally always empty, can keep as constexpr
																	constexpr std::array<uint32_t, E4BVariables::NUM_EXTRA_SAMPLE_PARAMETERS> params2{};
																	if (writer.writeType(params2.data(), sizeof(uint32_t) * params2.size()))
																	{
																		const std::vector sampleData(&sf2->samplesAsShort[shdr.start], &sf2->samplesAsShort[shdr.end]);
																		if (writer.writeType(sampleData.data(), sizeof(uint16_t) * sampleData.size()))
																		{
																			OutputDebugStringA("wrote sample data \n");
																		}
																	}
																}
															}
														}
													}
												}

												++sampleIndex;
											}
										}
									}

									E4EMSt emst;
									if (writer.writeType(E4BVariables::EOS_EMSt_TAG.data(), E4BVariables::EOS_EMSt_TAG.length()) && emst.write(writer))
									{
										OutputDebugStringA("wrote emst data \n");
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

	return false;
}

std::string BankConverter::ConvertNameToEmuName(const std::string_view& name) const
{
	std::string str(std::begin(name), std::ranges::find(name, '\0'));
	if (name.length() > E4BVariables::EOS_E4_MAX_NAME_LEN) { str.resize(E4BVariables::EOS_E4_MAX_NAME_LEN); }

	if(str.length() < E4BVariables::EOS_E4_MAX_NAME_LEN)
	{
		for(size_t i(str.length()); i < E4BVariables::EOS_E4_MAX_NAME_LEN; ++i)
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