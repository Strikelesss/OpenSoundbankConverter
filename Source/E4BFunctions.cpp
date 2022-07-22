#include "Header/E4BFunctions.h"
#include "Header/BinaryReader.h"
#include "Header/E4Preset.h"
#include "Header/E4Sample.h"

uint32_t E4BFunctions::GetSampleChannels(const E4Sample& sample)
{
	const auto& format(sample.GetFormat());
	if ((format & E4BVariables::EOS_STEREO_SAMPLE) == E4BVariables::EOS_STEREO_SAMPLE || 
		(format & E4BVariables::EOS_STEREO_SAMPLE_2) == E4BVariables::EOS_STEREO_SAMPLE_2) { return 2u; }

	return 1u;
}

bool E4BFunctions::ProcessE4BFile(BinaryReader& reader, E4Result& outResult)
{
	std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> tempChunkName{};
	reader.readType(tempChunkName.data(), E4BVariables::EOS_CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_FORM_TAG.data(), E4BVariables::EOS_FORM_TAG.length()) != 0) { return false; }

	reader.skipBytes(E4BVariables::EOS_CHUNK_NAME_LEN);

	reader.readType(tempChunkName.data(), E4BVariables::EOS_CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_E4_FORMAT_TAG.data(), E4BVariables::EOS_E4_FORMAT_TAG.length()) != 0) { return false; }

	reader.readType(tempChunkName.data(), E4BVariables::EOS_CHUNK_NAME_LEN);

	if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_TOC_TAG.data(), E4BVariables::EOS_TOC_TAG.length()) != 0) { return false; }

	uint32_t initialChunkLength(0u);
	reader.readType(&initialChunkLength);

	initialChunkLength = _byteswap_ulong(initialChunkLength);

	const uint32_t numChunks(initialChunkLength / E4BVariables::EOS_CHUNK_TOTAL_LEN);
	uint32_t lastLoc(0u);
	uint8_t currentSampleIndex(0ui8);

	for(uint32_t i(0); i < numChunks; ++i)
	{
		reader.readType(tempChunkName.data(), E4BVariables::EOS_CHUNK_NAME_LEN);

		uint32_t chunkLen(0u);
		reader.readTypeAtLocation(&chunkLen, reader.GetCurrentReadSize());

		chunkLen = _byteswap_ulong(chunkLen);

		uint32_t chunkLocation(0u);
		reader.readTypeAtLocation(&chunkLocation, reader.GetCurrentReadSize() + sizeof(uint32_t));

		chunkLocation = _byteswap_ulong(chunkLocation);
		const auto chunkLocationWithOffset(static_cast<size_t>(chunkLocation) + E4BVariables::EOS_CHUNK_NAME_OFFSET);

		lastLoc = chunkLen + static_cast<uint32_t>(chunkLocationWithOffset);

		if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()) == 0)
		{
			E4Preset preset;
			reader.readTypeAtLocation(&preset, chunkLocationWithOffset, PRESET_DATA_READ_SIZE);

			auto& presetResult(outResult.AddPreset(E4PresetResult(preset.GetPresetName())));
			if (preset.GetNumVoices() > 0u)
			{
				auto voicePos(chunkLocationWithOffset + static_cast<uint64_t>(preset.GetPresetDataSize()));

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

						auto zoneRange(numVoiceEnds > 1ull ? voiceEnd.GetZoneRange() : voice.GetZoneRange());

						// Account for the odd 'multisample' stuff
						if(numVoiceEnds > 1ull)
						{
							if(zoneRange.first == 0x00)
							{
								zoneRange.first = voice.GetZoneRange().first;
							}

							if (zoneRange.second == 0x7f)
							{
								zoneRange.second = voice.GetZoneRange().second;
							}
						}

						presetResult.AddVoice(E4VoiceResult(voice, std::move(zoneRange), voiceEnd.GetOriginalKey(), voiceEnd.GetSampleIndex()));
					}

					voicePos += static_cast<uint64_t>(voiceSize);
				}
			}
		}
		else
		{
			if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_E3_SAMPLE_TAG.data(), E4BVariables::EOS_E3_SAMPLE_TAG.length()) == 0)
			{
				E4Sample sample;
				reader.readTypeAtLocation(&sample, chunkLocationWithOffset - 4ull, SAMPLE_DATA_READ_SIZE);

				const auto wavStart(chunkLocation + E4BVariables::EOS_E3_SAMPLE_REDUNDANT_OFFSET);
				//const auto numSamples((chunkLength - E4BVariables::EOS_E3_SAMPLE_REDUNDANT_OFFSET) / 2);

				const auto& bankData(reader.GetData());
				std::vector<int16_t> convertedSampleData;
				std::vector<uint8_t> sampleData(&bankData[wavStart], &bankData[lastLoc]);

				for(size_t k(0); k < sampleData.size(); k += 2) { convertedSampleData.emplace_back(static_cast<int16_t>(sampleData[k] | sampleData[k + 1] << 8)); }

				// 0 = ???
				// 1 = start of left channel (SAMPLE_DATA_READ_SIZE)
				// 2 = start of right channel (0 if mono)
				// 3 = last sample of left channel (lastLoc - wavStart - 2 + SAMPLE_DATA_READ_SIZE)
				// 4 = last sample of right channel (3 - SAMPLE_DATA_READ_SIZE)
				// 5 = loop start (* 2 + SAMPLE_DATA_READ_SIZE)
				// 6 = 5 - SAMPLE_DATA_READ_SIZE
				// 7 = loop end (* 2 + SAMPLE_DATA_READ_SIZE)
				// 8 = 7 - SAMPLE_DATA_READ_SIZE

				uint32_t loopStart(0u);
				uint32_t loopEnd(0u);
				const auto canLoop((sample.GetFormat() & E4SampleVariables::SAMPLE_LOOP_FLAG) == E4SampleVariables::SAMPLE_LOOP_FLAG);
				if(canLoop)
				{
					const auto& params(sample.GetParams());
					loopStart = (params[5] - TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
					loopEnd = (params[7] - TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
				}

				outResult.MapSampleIndex(sample.GetIndex(), currentSampleIndex);
				outResult.AddSample(E4SampleResult(sample.GetName(), std::move(convertedSampleData), sample.GetSampleRate(), GetSampleChannels(sample), 
					canLoop, (sample.GetFormat() & E4SampleVariables::SAMPLE_RELEASE_FLAG) == E4SampleVariables::SAMPLE_RELEASE_FLAG, loopStart, loopEnd));

				++currentSampleIndex;
			}
			else
			{
				if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_E4_SEQ_TAG.data(), E4BVariables::EOS_E4_SEQ_TAG.length()) == 0)
				{
					E4Sequence seq;
					reader.readTypeAtLocation(&seq, chunkLocationWithOffset, SEQUENCE_DATA_READ_SIZE);

					const auto& bankData(reader.GetData());
					std::vector seqData(&bankData[chunkLocationWithOffset + E4BVariables::EOS_E4_MAX_NAME_LEN], &bankData[lastLoc]);

					outResult.AddSequence(E4SequenceResult(seq.GetName(), std::move(seqData)));
				}
			}
		}

		reader.skipBytes(E4BVariables::EOS_CHUNK_TOTAL_LEN - 4ull);
	}

	const auto& bankData(reader.GetData());
	if (lastLoc <= bankData.size())
	{
		reader.readTypeAtLocation(tempChunkName.data(), lastLoc, E4BVariables::EOS_CHUNK_NAME_LEN);
		if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_EMSt_TAG.data(), E4BVariables::EOS_EMSt_TAG.length()) != 0) { return false; }

		E4EMSt emst;
		reader.readTypeAtLocation(&emst, lastLoc + 4ull, EMST_DATA_READ_SIZE);

		outResult.SetCurrentPreset(emst.GetCurrentPreset());
	}

	return true;
}