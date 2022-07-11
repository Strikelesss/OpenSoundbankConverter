#include "Header/BankConverter.h"
#include "Header/E4Preset.h"
#include <fstream>
#include <iostream>
#include <sf2cute.hpp>

bool BankConverter::ConvertE4BToSF2(const E4Result& e4b, const std::string& bankName) const
{
	if(e4b.GetSamples().empty() || e4b.GetPresets().empty()) { return false; }

	sf2cute::SoundFont sf2;
	
	sf2.set_sound_engine("EMU8000");
	sf2.set_bank_name(bankName);
	sf2.set_rom_name("ROM");

	for(const auto& e4Sample : e4b.GetSamples())
	{
		sf2.NewSample(e4Sample.m_sampleName, e4Sample.m_sampleData, e4Sample.m_loopStart, e4Sample.m_loopEnd, e4Sample.m_sampleRate, 0, 0ui8);
	}

	uint32_t presetIndex(0u);
	for(const auto& preset : e4b.GetPresets())
	{
		uint32_t voiceIndex(0u);
		std::vector<sf2cute::SFPresetZone> presetZones;
		std::vector<sf2cute::SFInstrumentZone> instrumentZones;
		for(const auto& voice : preset.GetVoices())
		{
			const auto sampleIndex(voice.GetSampleIndex());
			if(sampleIndex < e4b.GetSamples().size())
			{
				const auto& e4Sample(e4b.GetSamples()[sampleIndex]);

				auto sampleMode(sf2cute::SampleMode::kNoLoop);
				if(e4Sample.m_isLooping)
				{
					sampleMode = sf2cute::SampleMode::kLoopContinuously;
				}
				else
				{
					if(e4Sample.m_isReleasing)
					{
						sampleMode = sf2cute::SampleMode::kLoopEndsByKeyDepression;
					}
					else
					{
						sampleMode = sf2cute::SampleMode::kNoLoop;
					}
				}

				const auto& zoneRange(voice.GetZoneRange());
				const auto& velRange(voice.GetVelocityRange());

				sf2cute::SFInstrumentZone instrumentZone(sf2.samples()[sampleIndex], std::vector{
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kOverridingRootKey, voice.GetOriginalKey()),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, static_cast<int16_t>(voice.GetPan() * 10i16)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialAttenuation, static_cast<int16_t>(voice.GetVolume() * 10i16)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kKeyRange, sf2cute::RangesType(zoneRange.first, zoneRange.second)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kVelRange, sf2cute::RangesType(velRange.first, velRange.second)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSampleModes, static_cast<int16_t>(sampleMode)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(voice.GetFineTune())),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, static_cast<int16_t>(voice.GetFilterFrequency())),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(voice.GetFilterQ())),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend, voice.GetChorusAmount())
				}, std::vector<sf2cute::SFModulatorItem>{});

				instrumentZones.emplace_back(instrumentZone);
				++voiceIndex;
			}
			else
			{
				return false;
			}
		}

		presetZones.emplace_back(sf2.NewInstrument(preset.GetName(), instrumentZones));
		const std::shared_ptr sfPreset(sf2.NewPreset(preset.GetName(), presetIndex, 0, presetZones));
		++presetIndex;
	}

	try 
	{
		std::ofstream ofs(std::filesystem::current_path().append(bankName + ".sf2"), std::ios::binary);
		sf2.Write(ofs);
		return true;
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
}