#include "Header/BankConverter.h"
#include "Header/BinaryWriter.h"
#include "Header/E4Preset.h"
#include "Header/E4Sample.h"
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

#include "Header/VoiceDefinitions.h"

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

bool BankConverter::ConvertE4BToSF2(const E4Result& e4b, const std::string_view& bankName, const ConverterOptions& options) const
{
	if(e4b.GetSamples().empty() || e4b.GetPresets().empty() || bankName.empty()) { return false; }

	const auto convertedName(ConvertNameToSFName(bankName));

	sf2cute::SoundFont sf2;
	
	sf2.set_sound_engine("EMU8000");
	sf2.set_bank_name(convertedName);
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
			const auto sampleIndex(e4b.GetSampleIndexMapping().at(voice.GetSampleIndex()));
			const auto& e4Sample(e4b.GetSamples()[sampleIndex]);

			uint16_t sampleMode(0ui16);
			if (e4Sample.IsLooping()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopContinuously); }
			if (e4Sample.IsReleasing()) { sampleMode |= static_cast<uint16_t>(sf2cute::SampleMode::kLoopEndsByKeyDepression); }

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

			const auto ampAttackSec(SF2Converter::secToTimecent(voice.GetAmpEnv().GetAttack1Sec()));
			if (ampAttackSec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackVolEnv, ampAttackSec)); }

			const auto ampDecayLevel(voice.GetAmpEnv().GetDecay1Level());
			const auto ampDecaySec(SF2Converter::secToTimecent(voice.GetAmpEnv().GetDecay1Sec()));
			if (ampDecayLevel < 100.f && ampDecaySec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayVolEnv, ampDecaySec)); }

			//const auto ampSustainLevel(voice.GetAmpEnv().GetDecay2Level());
			//const auto ampSustainSec(SF2Converter::secToTimecent(voice.GetAmpEnv().GetDecay2Sec()));
			//if(ampSustainLevel < 100.f && ampSustainSec > 0) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainVolEnv, ampSustainSec)); }

			const auto ampReleaseSec(SF2Converter::secToTimecent(voice.GetAmpEnv().GetRelease1Sec()));
			if (ampReleaseSec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseVolEnv, ampReleaseSec)); }

			const auto filterAttackSec(SF2Converter::secToTimecent(voice.GetFilterEnv().GetAttack1Sec()));
			if (filterAttackSec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kAttackModEnv, filterAttackSec)); }

			const auto filterDecayLevel(voice.GetFilterEnv().GetDecay1Level());
			const auto filterDecaySec(SF2Converter::secToTimecent(voice.GetFilterEnv().GetDecay1Sec()));
			if (filterDecayLevel < 100.f && filterDecaySec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kDecayModEnv, filterDecaySec)); }

			//const auto filterSustainLevel(voice.GetFilterEnv().GetDecay2Level());
			//const auto filterSustainSec(SF2Converter::secToTimecent(voice.GetFilterEnv().GetDecay2Sec()));
			//if(filterSustainLevel < 100.f && filterSustainSec > 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kSustainModEnv, filterSustainSec)); }

			const auto filterReleaseSec(SF2Converter::secToTimecent(voice.GetFilterEnv().GetRelease1Sec()));
			if (filterReleaseSec != 0i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kReleaseModEnv, filterReleaseSec)); }

			// Filters

			const auto filterFreqCents(SF2Converter::FilterFrequencyToCents(voice.GetFilterFrequency()));
			if (filterFreqCents >= 1500i16 && filterFreqCents < 13500i16) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterFc, filterFreqCents)); }

			const auto filterQ(voice.GetFilterQ());
			if (filterQ > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kInitialFilterQ, static_cast<int16_t>(filterQ * 10.f))); }

			// Amplifier / Oscillator

			const auto pan(options.m_flipPan ? -voice.GetPan() : voice.GetPan());
			if (pan != 0i8) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kPan, static_cast<int16_t>(pan * 10i16))); }

			const auto fineTune(voice.GetFineTune());
			if (fineTune != 0.) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kFineTune, static_cast<int16_t>(fineTune))); }

			const auto chorusSend(voice.GetChorusAmount());
			if (chorusSend > 0.f) { instrumentZone.SetGenerator(sf2cute::SFGeneratorItem(sf2cute::SFGenerator::kChorusEffectsSend, static_cast<int16_t>(chorusSend * 10.f))); }

			instrumentZones.emplace_back(instrumentZone);
			++voiceIndex;
		}

		presetZones.emplace_back(sf2.NewInstrument(preset.GetName(), instrumentZones));
		const std::shared_ptr sfPreset(sf2.NewPreset(preset.GetName(), presetIndex, 0, presetZones));
		++presetIndex;
	}

	try 
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
				if(writer.writeType(E4BVariables::EMU4_FORM_TAG.data(), E4BVariables::EMU4_FORM_TAG.length()))
				{
					const auto numPresets(sf2->presetNum);
					if(numPresets <= 0) { return false; }

					std::unordered_map<size_t, size_t> presetChunkLocations{}, sampleChunkLocations{};

					// E4B0 + TOC1 + LENGTH
					auto totalSize(static_cast<uint32_t>(E4BVariables::EMU4_E4_FORMAT_TAG.length() + E4BVariables::EMU4_TOC_TAG.length() + sizeof(uint32_t)));

					totalSize += numPresets * (E4BVariables::CONTENT_CHUNK_LEN + E4BVariables::CHUNK_NAME_OFFSET + TOTAL_PRESET_DATA_SIZE);

					uint32_t tocSize(numPresets * E4BVariables::CONTENT_CHUNK_LEN);

					const auto shdrs(sf2->shdrs);
					for (const auto& shdr : shdrs)
					{
						const uint32_t sampleSize((shdr.end - shdr.start) * static_cast<uint32_t>(sizeof(uint16_t)));
						if (sampleSize > 0u)
						{
							totalSize += E4BVariables::CONTENT_CHUNK_LEN + E4BVariables::CHUNK_NAME_OFFSET + TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize;
							tocSize += E4BVariables::CONTENT_CHUNK_LEN;
						}
					} 

					for(int i(0); i < numPresets; ++i)
					{
						const auto preset(sf2->presets[i]);
						totalSize += preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE);
					}

					// EMSt + LENGTH
					totalSize += static_cast<uint32_t>(E4BVariables::EMU4_EMSt_TAG.length());

					// todo: determine length of EMSt, below may be enough if all the other data is redundant (doubtful)
					totalSize += TOTAL_EMST_DATA_SIZE;

					// Byteswap since E4B requires it
					totalSize = _byteswap_ulong(totalSize);
					tocSize = _byteswap_ulong(tocSize);

					if(writer.writeType(&totalSize))
					{
						if(writer.writeType(E4BVariables::EMU4_E4_FORMAT_TAG.data(), E4BVariables::EMU4_E4_FORMAT_TAG.length()))
						{
							if(writer.writeType(E4BVariables::EMU4_TOC_TAG.data(), E4BVariables::EMU4_TOC_TAG.length()))
							{
								if(writer.writeType(&tocSize))
								{
									constexpr uint16_t redundant16(0ui16);

									/*
									 * Chunks
									 */

									for(int i(0); i < numPresets; ++i)
									{
										if(writer.writeType(E4BVariables::EMU4_E4_PRESET_TAG.data(), E4BVariables::EMU4_E4_PRESET_TAG.length()))
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
													if(writer.writeType(&presetNum) && writer.writeType(ConvertNameToEmuName(preset.presetName).c_str(), E4BVariables::E4_MAX_NAME_LEN)
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
											if (writer.writeType(E4BVariables::EMU4_E3_SAMPLE_TAG.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.length()))
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
															E4BVariables::E4_MAX_NAME_LEN) && writer.writeType(&redundant16))
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
										if (writer.writeTypeAtLocation(&currentPos, presetChunkLocations[i]) && writer.writeType(E4BVariables::EMU4_E4_PRESET_TAG.data(), E4BVariables::EMU4_E4_PRESET_TAG.length()))
										{
											const auto preset(sf2->presets[i]);
											uint32_t presetChunkLen(_byteswap_ulong(sizeof(uint16_t) + TOTAL_PRESET_DATA_SIZE + preset.regionNum * (VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
											if (writer.writeType(&presetChunkLen))
											{
												const auto presetNum(static_cast<uint16_t>(i));
												if (writer.writeType(&presetNum) && writer.writeType(ConvertNameToEmuName(preset.presetName).c_str(), E4BVariables::E4_MAX_NAME_LEN))
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
																			const auto region(preset.regions[j]);
																			const auto filterFreq(static_cast<int16_t>(tsf_cents2Hertz(static_cast<float>(region.initialFilterFc))));
																			const auto pan(static_cast<int8_t>(options.m_flipPan ? -region.pan : region.pan * 100.f));
																			const auto volume(static_cast<int8_t>(region.attenuation));
																			const auto fineTune(static_cast<double>(region.tune));
																			const auto filterQ(static_cast<float>(region.initialFilterQ) / 10.f);

																			const auto filterEnvPos(region.modEnvToFilterFc);
																			const auto filterEnvPosHz(tsf_cents2Hertz(std::abs(static_cast<float>(filterEnvPos))));

																			// TODO: find a good value for this
																			constexpr auto FILTER_FREQUENCY_MAX_HZ(3000.f); // 8372.22363f

																			float filterEnvPosPercent(0.f);
																			if(filterEnvPos > 0)
																			{
																				filterEnvPosPercent = filterEnvPosHz * 100.f / FILTER_FREQUENCY_MAX_HZ;
																			}
																			else if(filterEnvPos < 0)
																			{
																				filterEnvPosPercent = -filterEnvPosHz * 100.f / FILTER_FREQUENCY_MAX_HZ;
																			}

																			const auto ampEnv(region.ampenv);
																			const auto keyDelay(static_cast<double>(ampEnv.delay));
																			const auto ampAttack(static_cast<double>(ampEnv.attack));
																			const auto ampRelease(static_cast<double>(ampEnv.release));

																			const auto filterEnv(region.modenv);
																			const auto filterAttack(static_cast<double>(filterEnv.attack));
																			const auto filterRelease(static_cast<double>(filterEnv.release));

																			E4Voice voice(0.f, 0.f, filterFreq, pan, volume, fineTune, keyDelay, filterQ, { region.lokey, region.hikey },
																				{ region.lovel, region.hivel }, E4Envelope(ampAttack, ampRelease), E4Envelope(filterAttack, filterRelease), 
																				E4Cord(80ui8, 56ui8, VoiceDefinitions::ConvertPercentToByteF(filterEnvPosPercent)));

																			const auto originalKey(static_cast<uint8_t>(region.pitch_keycenter));
																			E4VoiceEndData voiceEnd(region.sampleIndex, originalKey);

																			if(!sampleModes.contains(region.sampleIndex)) { sampleModes[region.sampleIndex] = static_cast<uint8_t>(region.loop_mode); }

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
											if (writer.writeTypeAtLocation(&currentPos, sampleChunkLocations[sampleIndex]) && writer.writeType(E4BVariables::EMU4_E3_SAMPLE_TAG.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.length()))
											{
												const uint32_t sampleChunkLen(_byteswap_ulong(sizeof(uint16_t) + TOTAL_SAMPLE_DATA_READ_SIZE + sampleSize));
												if (writer.writeType(&sampleChunkLen))
												{
													const auto sampleIndexBS(_byteswap_ushort(sampleIndex));
													if (writer.writeType(&sampleIndexBS) && writer.writeType(ConvertNameToEmuName(shdr.sampleName).c_str(), E4BVariables::E4_MAX_NAME_LEN))
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

																// Release is on by default for E4B since there is a noticable 'pop' after if it isn't on
																uint32_t format(MONO_SAMPLE | SAMPLE_RELEASE_FLAG);

																const auto& mode(sampleModes[static_cast<uint8_t>(sampleIndex)]);
																if(mode == 1 || mode == 3) { format |= SAMPLE_LOOP_FLAG; }

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
									if (writer.writeType(E4BVariables::EMU4_EMSt_TAG.data(), E4BVariables::EMU4_EMSt_TAG.length()) && emst.write(writer))
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
	if (name.length() > E4BVariables::E4_MAX_NAME_LEN) { str.resize(E4BVariables::E4_MAX_NAME_LEN); }

	if(str.length() < E4BVariables::E4_MAX_NAME_LEN)
	{
		for(size_t i(str.length()); i < E4BVariables::E4_MAX_NAME_LEN; ++i)
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