#include "Header/BankConverter.h"
#include "Header/E4Preset.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sf2cute.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include <tchar.h>

int16_t SF2Converter::FilterFrequencyToCents(const uint16_t freq)
{
	const auto K(1200 / std::log10(2));
	const auto centOffset(static_cast<int16_t>(std::round(K * (std::log10(freq) - std::log10(440)))));
	return std::clamp(static_cast<int16_t>(BASE_CENT_VALUE + centOffset), SF2_FILTER_MIN_FREQ, SF2_FILTER_MAX_FREQ);
}

int16_t SF2Converter::secToTimecent(const double sec)
{
	return static_cast<int16_t>(std::lround(std::log(sec) / std::log(2.) * 1200.));
}

bool BankConverter::ConvertE4BToSF2(const E4Result& e4b, const std::string& bankName) const
{
	if(e4b.GetSamples().empty() || e4b.GetPresets().empty()) { return false; }

	sf2cute::SoundFont sf2;
	
	sf2.set_sound_engine("EMU8000");
	sf2.set_bank_name(bankName);
	sf2.set_rom_name("ROM");

	for(const auto& e4Sample : e4b.GetSamples())
	{
		sf2.NewSample(e4Sample.GetName(), e4Sample.GetData(), e4Sample.GetLoopStart(), e4Sample.GetLoopEnd(), e4Sample.GetSampleRate(), 0, 0ui8);
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
			if(sampleIndex < e4b.GetSamples().size()) // TODO: account for samples having an ID greater than the size
			{
				const auto& e4Sample(e4b.GetSamples()[sampleIndex]);

				uint16_t sampleMode(0ui16);
				if(e4Sample.IsLooping()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopContinuously); }
				if(e4Sample.IsReleasing()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopEndsByKeyDepression); }

				const auto& zoneRange(voice.GetZoneRange());
				const auto& velRange(voice.GetVelocityRange());

				sf2cute::SFInstrumentZone instrumentZone(sf2.samples()[sampleIndex], std::vector{
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kOverridingRootKey, voice.GetOriginalKey()),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialAttenuation, std::clamp(static_cast<int16_t>(std::abs(voice.GetVolume()) * 10i16), 0i16, 144i16)), // Using abs on volume since SF2 does not support negative attenuation
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kKeyRange, sf2cute::RangesType(zoneRange.first, zoneRange.second)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kVelRange, sf2cute::RangesType(velRange.first, velRange.second)),
					sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSampleModes, static_cast<int16_t>(sampleMode))
				}, std::vector<sf2cute::SFModulatorItem>{});

				// Envelope

				const auto ampAttack(SF2Converter::secToTimecent(voice.GetAmpEnv().GetAttack1Sec()));
				if(ampAttack > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackVolEnv, ampAttack)); }

				const auto ampDecay(SF2Converter::secToTimecent(voice.GetAmpEnv().GetDecay1Sec()));
				if(ampDecay > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayVolEnv, ampDecay)); }

				//const auto ampSustain(SF2Converter::secs_to_timecents(voice.GetAmpEnv().GetDecay2Sec()));
				//if(ampSustain > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainVolEnv, ampSustain)); }

				const auto ampRelease(SF2Converter::secToTimecent(voice.GetAmpEnv().GetRelease1Sec()));
				if(ampRelease > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseVolEnv, ampRelease)); }

				const auto filterAttack(SF2Converter::secToTimecent(voice.GetFilterEnv().GetAttack1Sec()));
				if(filterAttack > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackModEnv, filterAttack)); }

				const auto filterDecay(SF2Converter::secToTimecent(voice.GetFilterEnv().GetDecay1Sec()));
				if(filterDecay > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayModEnv, filterDecay)); }

				//const auto filterSustain(SF2Converter::secToTimecent(voice.GetFilterEnv().GetDecay2Sec()));
				//if(filterSustain > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainModEnv, filterSustain)); }

				const auto filterRelease(SF2Converter::secToTimecent(voice.GetFilterEnv().GetRelease1Sec()));
				if(filterRelease != 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseModEnv, filterRelease)); }

				// Filters

				const auto filterFreqCents(SF2Converter::FilterFrequencyToCents(voice.GetFilterFrequency()));
				if(filterFreqCents >= 1500i16 && filterFreqCents < 13500i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, filterFreqCents)); }

				const auto filterQ(voice.GetFilterQ());
				if(filterQ > 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(filterQ * 10.))); }

				// Amplifier / Oscillator

				const auto pan(voice.GetPan());
				if(pan != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, static_cast<int16_t>(pan * 10i16))); }

				const auto fineTune(voice.GetFineTune());
				if(fineTune != 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(fineTune))); }

				const auto chorusSend(voice.GetChorusAmount());
				if(chorusSend > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend, static_cast<int16_t>(chorusSend * 10.f))); }

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
		auto sf2Path(std::filesystem::path(bankName).wstring());
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
		}

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
