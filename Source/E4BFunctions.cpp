#include "Header/E4BFunctions.h"
#include "Header/BinaryReader.h"
#include "Header/E4Preset.h"
#include "Header/E4BVariables.h"
#include "Header/WAVDefinitions.h"
#include "Header/E4Sample.h"

uint32_t E4BFunctions::GetSampleChannels(const E4Sample& sample)
{
	if ((sample.m_format & E4BVariables::STEREO_SAMPLE) == E4BVariables::STEREO_SAMPLE || 
		(sample.m_format & E4BVariables::STEREO_SAMPLE_2) == E4BVariables::STEREO_SAMPLE_2) { return 2u; }

	return 1u;
}

bool E4BFunctions::ProcessE4BFile(BinaryReader& reader, const EExtractionType extType, E4Result& outResult)
{
	std::array<char, 4> tempChunkName{};
	reader.readType(tempChunkName.data(), 4);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_FORM_TAG.data(), E4BVariables::EMU4_FORM_TAG.length()) != 0) { return false; }

	reader.skipBytes(4);

	reader.readType(tempChunkName.data(), 4);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E4_FORMAT.data(), E4BVariables::EMU4_E4_FORMAT.length()) != 0) { return false; }

	reader.readType(tempChunkName.data(), 4);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_TOC_TAG.data(), E4BVariables::EMU4_TOC_TAG.length()) != 0) { return false; }

	uint32_t initialChunkLength(0u);
	reader.readType(&initialChunkLength);

	initialChunkLength = _byteswap_ulong(initialChunkLength);

	const uint32_t numChunks(initialChunkLength / E4BVariables::CONTENT_CHUNK_LEN);
	uint32_t lastLoc(0u);
	for(uint32_t i(0); i < numChunks; ++i)
	{
		reader.readType(tempChunkName.data(), 4);

		if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E4_PRESET_TAG.data(), E4BVariables::EMU4_E4_PRESET_TAG.length()) == 0)
		{
			uint32_t chunkLen(0u);
			reader.readTypeAtLocation(&chunkLen, reader.GetCurrentReadSize());

			chunkLen = _byteswap_ulong(chunkLen);

			uint32_t chunkLocation(0u);
			reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + 4);

			chunkLocation = _byteswap_ulong(chunkLocation);

			lastLoc = chunkLen + chunkLocation + E4BVariables::EMU4_E3_SAMPLE_OFFSET;

			E4Preset preset;
			reader.readTypeAtLocation(&preset, chunkLocation + 10ull);

			auto& presetResult(outResult.m_presets.emplace_back(preset.m_name.data()));
			if (preset.GetNumVoices() > 0u)
			{
				auto voicePos(chunkLocation + sizeof(E4Preset) + 10);

				for (uint32_t j(1u); j <= preset.GetNumVoices(); ++j)
				{
					E4Voice voice;
					reader.readTypeAtLocation(&voice, voicePos);

					// Account for total voice size near the start of the voice
					const auto voiceSize(_byteswap_ushort(*reinterpret_cast<const unsigned short*>(voice.m_totalVoiceSize.data()))
						+ 9); // Add the 9 redundant bytes at the end

					const auto numVoiceEnds(static_cast<int>(voiceSize - sizeof(E4Voice)) / 22);
					for (int k(0); k < numVoiceEnds; ++k)
					{
						E4VoiceEndData voiceEnd;
						reader.readTypeAtLocation(&voiceEnd, voicePos + sizeof(E4Voice) + static_cast<unsigned long long>(k * 22));

						auto zoneRange(numVoiceEnds > 1 ? voiceEnd.GetZoneRange() : voice.GetZoneRange());

						// Account for the odd 'multisample' stuff
						if(numVoiceEnds > 1)
						{
							if(zoneRange.first == 0)
							{
								zoneRange.first = voice.GetZoneRange().first;
							}

							if (zoneRange.second == 127)
							{
								zoneRange.second = voice.GetZoneRange().second;
							}
						}

						presetResult.m_voices.emplace_back(voiceEnd.GetSampleIndex() - 1ui8, voiceEnd.GetOriginalKey(), voice.GetChorusWidth(), voice.GetChorusAmount(), voice.GetFilterFrequency(),
							voice.GetPan(), voice.GetVolume(), voice.GetFineTune(), voice.GetFilterQ(), voice.GetFilterType(), zoneRange, voice.GetVelocityRange());
					}

					voicePos += static_cast<unsigned long long>(voiceSize - 9); // Remove the 9 redundant bytes
				}
			}
		}
		else
		{
			if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.data(), E4BVariables::EMU4_E3_SAMPLE_TAG.length()) == 0)
			{
				uint32_t chunkLen(0u);
				reader.readTypeAtLocation(&chunkLen, reader.GetCurrentReadSize());

				chunkLen = _byteswap_ulong(chunkLen);

				uint32_t chunkLocation(0u);
				reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + 4);

				chunkLocation = _byteswap_ulong(chunkLocation);

				lastLoc = chunkLen + chunkLocation + E4BVariables::EMU4_E3_SAMPLE_OFFSET;

				E4Sample sample;
				reader.readTypeAtLocation(&sample, chunkLocation + 10ull);

				const auto wavStart(chunkLocation + E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET);
				//const auto numSamples((chunkLength - E4BVariables::EMU4_E3_SAMPLE_REDUNDANT_OFFSET) / 2);

				uint32_t loopStart(0u);
				uint32_t loopEnd(0u);
				const auto canLoop(sample.m_format & SAMPLE_LOOP_FLAG);
				if(canLoop)
				{
					loopStart = (sample.m_params[5] - SAMPLE_LOOP_OFFSET) / 2u;
					loopEnd = (sample.m_params[7] - SAMPLE_LOOP_OFFSET) / 2u;
				}

				const auto& bankData(reader.GetData());

				std::vector<int16_t> convertedSampleData;
				std::vector<uint8_t> sampleData(&bankData[wavStart], &bankData[lastLoc]);

				for(size_t k(0); k < sampleData.size(); k += 2) { convertedSampleData.emplace_back(static_cast<int16_t>(sampleData[k] | sampleData[k + 1] << 8)); }

				outResult.m_samples.emplace_back(std::string(sample.m_name), std::move(convertedSampleData), sample.m_sample_rate, GetSampleChannels(sample), 
					canLoop, sample.m_format & SAMPLE_RELEASE_FLAG, loopStart, loopEnd);
			}
		}

		reader.skipBytes(E4BVariables::CONTENT_CHUNK_LEN - 4ull);
	}

	const auto& bankData(reader.GetData());
	if (lastLoc <= bankData.size())
	{
		E4Emst emst;
		reader.readTypeAtLocation(&emst, lastLoc);

		if (std::strcmp(emst.m_name.data(), E4BVariables::EMU4_EMSt_TAG.data()) == 0)
		{
			outResult.m_currentPreset = emst.GetCurrentPreset();
		}
	}

	return true;
}
