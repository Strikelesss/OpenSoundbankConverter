#include "Header/E4BFunctions.h"
#include "Header/E4Preset.h"
#include "Header/E4BVariables.h"
#include "Header/WAVDefinitions.h"
#include "Header/Chunk.h"
#include "Header/E3Sample.h"

bool E4BFunctions::replaceStringOccurrance(std::string& str, const std::string& from, const std::string& to)
{
	const size_t start_pos = str.find(from);
	if (start_pos == std::string::npos) { return false; }
	str.replace(start_pos, from.length(), to);
	return true;
}

int E4BFunctions::VerifyChunkName(const Chunk* chunk, const char* name)
{
	const int v(std::strncmp(chunk->name.data(), name, E4BVariables::CHUNK_NAME_LEN));
	if (v) { std::printf("Unexpected chunk name %s\n", name); }
	return v;
}

int E4BFunctions::GetSampleChannels(const E3Sample* sample)
{
	if ((sample->format & E4BVariables::STEREO_SAMPLE) == E4BVariables::STEREO_SAMPLE || 
		(sample->format & E4BVariables::STEREO_SAMPLE_2) == E4BVariables::STEREO_SAMPLE_2) { return 2; }

	return 1;
}

void E4BFunctions::ExtractSampleData(E3Sample* sample, const unsigned int len)
{
	short frame;
	const int channels = GetSampleChannels(sample);

	//We divide between the bytes per frame (number of channels * 2 bytes)
	//The 16 or 8 bytes are the 4 or 8 short int used for padding.
	const size_t nframes = (len - sizeof(E3Sample) - static_cast<size_t>(8 * channels)) / static_cast<size_t>(2 * channels);

	std::printf("Sample size: %d; frames: %llu; channels: %s\n", len, nframes, channels == 1 ? "mono" : "stereo");

	short* l_channel = sample->frames + 2;
	for (int i = 0; i < nframes; i++)
	{
		frame = *l_channel;
		l_channel++;
	}
}

bool E4BFunctions::ProcessE4BFile(std::vector<char>& bankData, const EExtractionType extType, E4Result& outResult, const std::filesystem::path& wavFileFormat)
{
	auto chunkData = reinterpret_cast<Chunk*>(bankData.data());
	if (VerifyChunkName(chunkData, E4BVariables::EMU4_FORM_TAG.data())) { return false; }

	if (strncmp(chunkData->data, E4BVariables::EMU4_E4_FORMAT.data(), E4BVariables::EMU4_E4_FORMAT.length()) != 0)
	{
		std::printf("Unexpected format\n"); return false;
	}

	chunkData = reinterpret_cast<Chunk*>(&chunkData->data[E4BVariables::EMU4_E4_FORMAT.length()]);
	if (VerifyChunkName(chunkData, E4BVariables::EMU4_TOC_TAG.data())) { return false; }

	const uint32_t chunk_len(_byteswap_ulong(chunkData->len));
	auto content_chunk(reinterpret_cast<Chunk*>(chunkData->data));

	auto endOfLast(0ul);
	uint32_t i(0u);
	uint32_t sample_index(0u);
	while (i < chunk_len)
	{
		const auto chunkLength(_byteswap_ulong(content_chunk->len) + E4BVariables::EMU4_E3_SAMPLE_OFFSET);
		const auto chunkStart(_byteswap_ulong(*reinterpret_cast<unsigned int*>(content_chunk->data)));
		const auto chunkStartData(reinterpret_cast<Chunk*>(&bankData[chunkStart]));
		endOfLast = chunkLength + chunkStart; // this is WAV end as well (was -1 before, not sure if it was needed)

		//std::printf("%.4s: %.16s \n", content_chunk->name.data(), &content_chunk->data[6]);

		if (std::strcmp(content_chunk->name.data(), E4BVariables::EMU4_E4_PRESET_TAG.data()) == 0
			&& extType == EExtractionType::PRINT_INFO)
		{
			const auto preset(reinterpret_cast<E4Preset*>(&chunkStartData->data[2]));
			auto& presetResult(outResult.m_presets.emplace_back(preset->m_name.data()));

			if (preset->GetNumVoices() > 0)
			{
				auto* voice(reinterpret_cast<E4Voice*>(&preset->m_data[0]));
				for (uint32_t j(1u); j <= preset->GetNumVoices(); ++j)
				{
					std::string_view filterType;
					if (voice->GetFilterType(filterType))
					{
						presetResult.m_voices.emplace_back(voice->GetOriginalKey(), voice->GetFilterFrequency(), voice->GetPan(), voice->GetVolume(),
							voice->GetFineTune(), filterType, voice->GetZoneRange());

						/*
						const auto zoneRange(voice->GetZoneRange());
						std::printf("voice %u: orig key: %u, zone: %u-%u, pan: %d, fine tune: %f, filter name: %s, filter freq: %u \n", j, 
							voice->GetOriginalKey(), zoneRange.first, zoneRange.second, voice->GetPan(),
							voice->GetFineTune(), filterType.data(), voice->GetFilterFrequency());
							*/
					}

					// Account for total voice size near the start of the voice
					const auto voiceSize(_byteswap_ushort(*reinterpret_cast<unsigned short*>(voice->m_totalVoiceSize.data())));
					voice = reinterpret_cast<E4Voice*>(&voice->m_data[voiceSize - TOTAL_VOICE_EXTRACTION_SIZE]);
				}
			}
		}
		else
		{
			if (std::strcmp(content_chunk->name.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.data()) == 0
				&& extType == EExtractionType::WAV_REPLACEMENT)
			{
				const auto sample(reinterpret_cast<E3Sample*>(&chunkStartData->data[2]));
				const auto wavStart(chunkStart + E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET);
				const auto numSamples((chunkLength - E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET) / 2);

				std::string sampleIndexFormatted;
				if (sample_index >= 100)
				{
					sampleIndexFormatted = std::to_string(sample_index);
				}
				else
				{
					if (sample_index >= 10)
					{
						sampleIndexFormatted = std::to_string(0).append(std::to_string(sample_index));
					}
					else
					{
						sampleIndexFormatted = std::to_string(0).append(std::to_string(0)).append(std::to_string(sample_index));
					}
				}

				auto replacementFormat(wavFileFormat.string());
				replaceStringOccurrance(replacementFormat, "XXX", sampleIndexFormatted);
				std::filesystem::path replacementPath(replacementFormat);
				replacementPath = replacementPath.lexically_normal();

				std::vector<unsigned char> wavData;
				if (exists(replacementPath) && WAVDefinitions::LoadWavExtensibleFile(replacementPath, wavData))
				{
					std::printf("%c %c", static_cast<unsigned char>(-125), static_cast<unsigned char>(131));
				}

				ExtractSampleData(sample, _byteswap_ulong(chunkStartData->len));
				sample_index++;
			}
		}

		content_chunk = reinterpret_cast<Chunk*>(&content_chunk->data[E4BVariables::CONTENT_CHUNK_DATA_LEN]);
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
