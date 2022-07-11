#include "Header/E4BFunctions.h"

#include <cassert>

#include "Header/E4Preset.h"
#include "Header/E4BVariables.h"
#include "Header/WAVDefinitions.h"
#include "Header/Chunk.h"
#include "Header/E3Sample.h"

int32_t E4BFunctions::VerifyChunkName(const Chunk* chunk, const char* name)
{
	const int32_t v(std::strncmp(chunk->name.data(), name, E4BVariables::CHUNK_NAME_LEN));
	if (v) { std::printf("Unexpected chunk name %s\n", name); }
	return v;
}

uint32_t E4BFunctions::GetSampleChannels(const E3Sample* sample)
{
	if ((sample->m_format & E4BVariables::STEREO_SAMPLE) == E4BVariables::STEREO_SAMPLE || 
		(sample->m_format & E4BVariables::STEREO_SAMPLE_2) == E4BVariables::STEREO_SAMPLE_2) { return 2u; }

	return 1u;
}

bool E4BFunctions::ProcessE4BFile(std::vector<char>& bankData, const EExtractionType extType, E4Result& outResult)
{
	auto* pos(bankData.data());

	auto chunkData(reinterpret_cast<Chunk*>(bankData.data()));
	if (VerifyChunkName(chunkData, E4BVariables::EMU4_FORM_TAG.data())) { return false; }

	chunkData = std::next(chunkData, 1);
	pos += sizeof(Chunk);

	if (strncmp(chunkData->name.data(), E4BVariables::EMU4_E4_FORMAT.data(), E4BVariables::EMU4_E4_FORMAT.length()) != 0)
	{
		std::printf("Unexpected format\n"); return false;
	}

	chunkData = reinterpret_cast<Chunk*>(&pos[E4BVariables::EMU4_E4_FORMAT.length()]);
	if (VerifyChunkName(chunkData, E4BVariables::EMU4_TOC_TAG.data())) { return false; }

	const uint32_t chunk_len(_byteswap_ulong(chunkData->len));
	chunkData = std::next(chunkData, 1);

	auto endOfLast(0ul);
	uint32_t i(0u);
	uint32_t sample_index(0u);
	while (i < chunk_len)
	{
		const auto chunkLength(_byteswap_ulong(chunkData->len) + E4BVariables::EMU4_E3_SAMPLE_OFFSET);
		const auto chunkStart(_byteswap_ulong(*reinterpret_cast<unsigned int*>(&chunkData[1])));

		endOfLast = chunkLength + chunkStart; // this is WAV end as well (was -1 before, not sure if it was needed)
		const auto chunkStartData = reinterpret_cast<Chunk*>(&bankData[chunkStart + 2ull]);

		//std::printf("%.4s: %.16s \n", content_chunk->name.data(), &content_chunk->data[6]);

		if (std::strcmp(chunkData->name.data(), E4BVariables::EMU4_E4_PRESET_TAG.data()) == 0
			&& extType == EExtractionType::PRINT_INFO)
		{
			const auto preset(reinterpret_cast<E4Preset*>(&chunkStartData[1]));
			auto& presetResult(outResult.m_presets.emplace_back(preset->m_name.data()));

			if (preset->GetNumVoices() > 0)
			{
				auto voicePosition(chunkStart + sizeof(E4Preset) + 10);
				auto* voice(reinterpret_cast<E4Voice*>(&bankData[voicePosition]));
				for (uint32_t j(1u); j <= preset->GetNumVoices(); ++j)
				{
					// Account for total voice size near the start of the voice
					const auto voiceSize(_byteswap_ushort(*reinterpret_cast<unsigned short*>(voice->m_totalVoiceSize.data()))
						+ 9); // Add the 9 redundant bytes at the end

					const auto numVoiceEnds(static_cast<int>(voiceSize - sizeof(E4Voice)) / 22);
					for (int k(0); k < numVoiceEnds; ++k)
					{
						const auto* voiceEnd(reinterpret_cast<E4VoiceEndData*>(&bankData[voicePosition + sizeof(E4Voice) + static_cast<unsigned long long>(k * 22)]));
						auto zoneRange(numVoiceEnds > 1 ? voiceEnd->GetZoneRange() : voice->GetZoneRange());

						// Account for the odd 'multisample' stuff
						if(numVoiceEnds > 1)
						{
							if(zoneRange.first == 0)
							{
								zoneRange.first = voice->GetZoneRange().first;
							}

							if (zoneRange.second == 127)
							{
								zoneRange.second = voice->GetZoneRange().second;
							}
						}

						presetResult.m_voices.emplace_back(voiceEnd->GetSampleIndex() - 1ui8, voiceEnd->GetOriginalKey(), voice->GetChorusWidth(), voice->GetChorusAmount(), voice->GetFilterFrequency(),
							voice->GetPan(), voice->GetVolume(), voice->GetFineTune(), voice->GetFilterQ(), voice->GetFilterType(), zoneRange, voice->GetVelocityRange());
					}

					voicePosition += static_cast<unsigned long long>(voiceSize - 9); // Remove the 9 redundant bytes
					voice = reinterpret_cast<E4Voice*>(&bankData[voicePosition]);
				}
			}
		}
		else
		{
			if (std::strcmp(chunkData->name.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.data()) == 0)
			{
				const auto sample(reinterpret_cast<E3Sample*>(&chunkStartData[1]));
				const auto wavStart(chunkStart + E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET);
				//const auto numSamples((chunkLength - E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET) / 2);

				uint32_t loopStart(0u);
				uint32_t loopEnd(0u);
				const auto canLoop(sample->m_format & SAMPLE_LOOP_FLAG);
				if(canLoop)
				{
					loopStart = (sample->m_params[5] - 92u) / 2u;
					loopEnd = (sample->m_params[7] - 92u) / 2u;
				}

				std::vector<int16_t> convertedSampleData;
				std::vector<uint8_t> sampleData(&bankData[wavStart], &bankData[endOfLast]);

				for(size_t k(0); k < sampleData.size(); k += 2) { convertedSampleData.emplace_back(static_cast<int16_t>(sampleData[k] | sampleData[k + 1] << 8)); }

				outResult.m_samples.emplace_back(std::string(sample->m_name), std::move(convertedSampleData), sample->m_sample_rate, GetSampleChannels(sample), 
					canLoop, sample->m_format & SAMPLE_RELEASE_FLAG, loopStart, loopEnd);

				sample_index++;
			}
		}

		chunkData = std::next(chunkData, E4BVariables::CONTENT_CHUNK_LEN / sizeof(Chunk));
		i += E4BVariables::CONTENT_CHUNK_LEN;
	}

	if (endOfLast <= bankData.size())
	{
		const auto* emst(reinterpret_cast<E4Emst*>(&bankData[endOfLast]));
		if (std::strcmp(emst->m_name.data(), E4BVariables::EMU4_EMSt_TAG.data()) == 0)
		{
			outResult.m_currentPreset = emst->GetCurrentPreset();
		}
	}

	return true;
}
