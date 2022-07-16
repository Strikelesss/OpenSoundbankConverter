#include "Header/E4BFunctions.h"
#include <fstream>
#include "Header/BinaryReader.h"
#include "Header/E4Preset.h"
#include "Header/E4Sample.h"

uint32_t E4BFunctions::GetSampleChannels(const E4Sample& sample)
{
	if ((sample.m_format & E4BVariables::STEREO_SAMPLE) == E4BVariables::STEREO_SAMPLE || 
		(sample.m_format & E4BVariables::STEREO_SAMPLE_2) == E4BVariables::STEREO_SAMPLE_2) { return 2u; }

	return 1u;
}

bool E4BFunctions::ProcessE4BFile(BinaryReader& reader, E4Result& outResult)
{
	std::array<char, E4BVariables::CHUNK_NAME_LEN> tempChunkName{};
	reader.readType(tempChunkName.data(), E4BVariables::CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_FORM_TAG.data(), E4BVariables::EMU4_FORM_TAG.length()) != 0) { return false; }

	reader.skipBytes(E4BVariables::CHUNK_NAME_LEN);

	reader.readType(tempChunkName.data(), E4BVariables::CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E4_FORMAT_TAG.data(), E4BVariables::EMU4_E4_FORMAT_TAG.length()) != 0) { return false; }

	reader.readType(tempChunkName.data(), E4BVariables::CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_TOC_TAG.data(), E4BVariables::EMU4_TOC_TAG.length()) != 0) { return false; }

	uint32_t initialChunkLength(0u);
	reader.readType(&initialChunkLength);

	initialChunkLength = _byteswap_ulong(initialChunkLength);

	const uint32_t numChunks(initialChunkLength / E4BVariables::CONTENT_CHUNK_LEN);
	uint32_t lastLoc(0u);
	for(uint32_t i(0); i < numChunks; ++i)
	{
		reader.readType(tempChunkName.data(), E4BVariables::CHUNK_NAME_LEN);

		if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E4_PRESET_TAG.data(), E4BVariables::EMU4_E4_PRESET_TAG.length()) == 0)
		{
			uint32_t chunkLen(0u);
			reader.readTypeAtLocation(&chunkLen, reader.GetCurrentReadSize());

			chunkLen = _byteswap_ulong(chunkLen);

			uint32_t chunkLocation(0u);
			reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + E4BVariables::CHUNK_NAME_LEN);

			chunkLocation = _byteswap_ulong(chunkLocation);

			lastLoc = chunkLen + chunkLocation + E4BVariables::EMU4_E3_SAMPLE_OFFSET;

			E4Preset preset;
			reader.readTypeAtLocation(&preset, chunkLocation + E4BVariables::CHUNK_NAME_OFFSET, PRESET_DATA_READ_SIZE);

			auto& presetResult(outResult.AddPreset(E4PresetResult(preset.GetPresetName())));
			if (preset.GetNumVoices() > 0u)
			{
				auto voicePos(chunkLocation + static_cast<uint64_t>(preset.GetPresetDataSize()) + E4BVariables::CHUNK_NAME_OFFSET);

				for (uint32_t j(1u); j <= preset.GetNumVoices(); ++j)
				{
					E4Voice voice;
					reader.readTypeAtLocation(&voice, voicePos, VOICE_DATA_READ_SIZE);

					const auto voiceSize(voice.GetVoiceDataSize());

					const auto numVoiceEnds((voiceSize - VOICE_DATA_SIZE) / VOICE_END_DATA_SIZE);
					for (uint64_t k(0ull); k < numVoiceEnds; ++k)
					{
						E4VoiceEndData voiceEnd;
						reader.readTypeAtLocation(&voiceEnd, voicePos + VOICE_DATA_SIZE + k * VOICE_END_DATA_SIZE, VOICE_END_DATA_READ_SIZE);

						auto zoneRange(numVoiceEnds > 1 ? voiceEnd.GetZoneRange() : voice.GetZoneRange());

						// Account for the odd 'multisample' stuff
						if(numVoiceEnds > 1ull)
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

						presetResult.AddVoice(E4VoiceResult(voiceEnd.GetSampleIndex() - 1ui8, voiceEnd.GetOriginalKey(), voice.GetChorusWidth(), voice.GetChorusAmount(), 
							voice.GetFilterFrequency(), voice.GetPan(), voice.GetVolume(), voice.GetFineTune(), voice.GetFilterQ(), voice.GetFilterType(), zoneRange, 
							voice.GetVelocityRange(), voice.GetAmpEnv(), voice.GetFilterEnv(), voice.GetAuxEnv()));
					}

					voicePos += static_cast<unsigned long long>(voiceSize);
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
				reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + E4BVariables::CHUNK_NAME_LEN);

				chunkLocation = _byteswap_ulong(chunkLocation);

				lastLoc = chunkLen + chunkLocation + E4BVariables::EMU4_E3_SAMPLE_OFFSET;

				E4Sample sample;
				reader.readTypeAtLocation(&sample, chunkLocation + E4BVariables::CHUNK_NAME_OFFSET, SAMPLE_DATA_READ_SIZE);

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

				outResult.AddSample(E4SampleResult(std::string(sample.m_name), std::move(convertedSampleData), sample.m_sample_rate, GetSampleChannels(sample), 
					canLoop, sample.m_format & SAMPLE_RELEASE_FLAG, loopStart, loopEnd));
			}
			else
			{
				if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_E4_SEQ_TAG.data(), E4BVariables::EMU4_E4_SEQ_TAG.length()) == 0)
				{
					uint32_t chunkLen(0u);
					reader.readTypeAtLocation(&chunkLen, reader.GetCurrentReadSize());

					chunkLen = _byteswap_ulong(chunkLen);

					uint32_t chunkLocation(0u);
					reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + E4BVariables::CHUNK_NAME_LEN);

					chunkLocation = _byteswap_ulong(chunkLocation);

					lastLoc = chunkLen + chunkLocation + E4BVariables::EMU4_E3_SAMPLE_OFFSET;

					E4Sequence seq;
					reader.readTypeAtLocation(&seq, chunkLocation + E4BVariables::CHUNK_NAME_OFFSET);

					const auto& bankData(reader.GetData());
					std::vector seqData(&bankData[chunkLocation + E4BVariables::CHUNK_NAME_OFFSET + E4BVariables::NAME_SIZE], &bankData[lastLoc]);

					outResult.AddSequence(E4SequenceResult(seq.GetName(), std::move(seqData)));
				}
			}
		}

		reader.skipBytes(E4BVariables::CONTENT_CHUNK_LEN - 4ull);
	}

	const auto& bankData(reader.GetData());
	if (lastLoc <= bankData.size())
	{
		reader.readTypeAtLocation(tempChunkName.data(), lastLoc, E4BVariables::CHUNK_NAME_LEN);
		if (std::strncmp(tempChunkName.data(), E4BVariables::EMU4_EMSt_TAG.data(), E4BVariables::EMU4_EMSt_TAG.length()) != 0) { return false; }

		E4Emst emst;
		reader.readTypeAtLocation(&emst, lastLoc + 4ull);

		outResult.SetCurrentPreset(emst.GetCurrentPreset());
	}

	return true;
}