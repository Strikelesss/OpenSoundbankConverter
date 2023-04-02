#include "Header/E4BFunctions.h"
#include "Header/BinaryReader.h"
#include "Header/E4Result.h"
#include "Header/E4Sample.h"
#include "Header/Logger.h"

uint32_t E4BFunctions::GetSampleChannels(const E4Sample& sample)
{
	const auto& format(sample.GetFormat());
	if ((format & E4SampleVariables::EOS_STEREO_SAMPLE) == E4SampleVariables::EOS_STEREO_SAMPLE || 
		(format & E4SampleVariables::EOS_STEREO_SAMPLE_2) == E4SampleVariables::EOS_STEREO_SAMPLE_2) { return 2u; }

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

	for(uint32_t i(0u); i < numChunks; ++i)
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
			reader.readTypeAtLocation(&preset, chunkLocationWithOffset - 2ull, PRESET_DATA_READ_SIZE);

			// This will stay here since we still want to create presets even though they have no data
			auto& presetResult(outResult.AddPreset(E4PresetResult(preset.GetIndex(), preset.GetName())));

			const auto numVoices(preset.GetNumVoices());
			if (numVoices > 0ui16)
			{
				auto voicePos(chunkLocationWithOffset + static_cast<uint64_t>(preset.GetDataSize()));

				for (uint16_t j(0ui16); j < numVoices; ++j)
				{
					E4Voice voice;
					reader.readTypeAtLocation(&voice, voicePos, VOICE_DATA_READ_SIZE);

					const auto voiceSize(voice.GetVoiceDataSize());

					const auto numVoiceEnds((voiceSize - VOICE_DATA_SIZE) / VOICE_END_DATA_SIZE);
					for (uint64_t k(0ull); k < numVoiceEnds; ++k)
					{
						E4VoiceEndData voiceEnd;
						reader.readTypeAtLocation(&voiceEnd, voicePos + VOICE_DATA_SIZE + k * VOICE_END_DATA_SIZE, VOICE_END_DATA_READ_SIZE);

						const auto multipleVoiceEnds(numVoiceEnds > 1ull);
						auto keyZoneRange(multipleVoiceEnds ? voiceEnd.GetKeyZoneRange() : voice.GetKeyZoneRange());
						const auto pan(multipleVoiceEnds ? voiceEnd.GetPan() : voice.GetPan());
						const auto volume(multipleVoiceEnds ? voiceEnd.GetVolume() : voice.GetVolume());
						const auto fineTune(multipleVoiceEnds ? voiceEnd.GetFineTune() : voice.GetFineTune());

						// Account for the odd 'multisample' stuff
						if (multipleVoiceEnds)
						{
							if (keyZoneRange.GetLow() == E4BVariables::EOS_MIN_KEY_ZONE_RANGE) { keyZoneRange.SetLow(voice.GetKeyZoneRange().GetLow()); }
							if (keyZoneRange.GetHigh() == E4BVariables::EOS_MAX_KEY_ZONE_RANGE) { keyZoneRange.SetHigh(voice.GetKeyZoneRange().GetHigh()); }
						}

						presetResult.AddVoice(E4VoiceResult(voice, keyZoneRange, voiceEnd.GetOriginalKey(), 
							voiceEnd.GetSampleIndex(), volume, pan, fineTune));
					}

					voicePos += static_cast<uint64_t>(voiceSize);
				}
			}
		}
		else
		{
			if (std::strncmp(tempChunkName.data(), E4BVariables::EOS_E3_SAMPLE_TAG.data(), E4BVariables::EOS_E3_SAMPLE_TAG.length()) == 0)
			{
				const auto sampleDataLocation(chunkLocationWithOffset - (sizeof(uint16_t) + sizeof(uint16_t)));

				E4Sample sample;
				reader.readTypeAtLocation(&sample, sampleDataLocation, SAMPLE_DATA_READ_SIZE);

				const auto wavStart(sampleDataLocation + 4ull + TOTAL_SAMPLE_DATA_READ_SIZE);

				const auto& bankData(reader.GetData());
				std::vector<int16_t> convertedSampleData;
				std::vector<uint8_t> sampleData(&bankData[wavStart], &bankData[lastLoc]);

				for(size_t k(0); k < sampleData.size(); k += 2) { convertedSampleData.emplace_back(static_cast<int16_t>(sampleData[k] | sampleData[k + 1] << 8)); }

				const auto sampleFormat(sample.GetFormat());

				uint32_t loopStart(0u);
				uint32_t loopEnd(0u);
				const auto canLoop((sampleFormat & E4SampleVariables::SAMPLE_LOOP_FLAG) == E4SampleVariables::SAMPLE_LOOP_FLAG);
				if(canLoop)
				{
					const auto& params(sample.GetParams());
					if(params[E4SampleVariables::SAMPLE_LOOP_START_PARAM_INDEX] > TOTAL_SAMPLE_DATA_READ_SIZE)
					{
						loopStart = (params[E4SampleVariables::SAMPLE_LOOP_START_PARAM_INDEX] - TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
					}

					if(params[E4SampleVariables::SAMPLE_LOOP_END_PARAM_INDEX] > TOTAL_SAMPLE_DATA_READ_SIZE)
					{
						loopEnd = (params[E4SampleVariables::SAMPLE_LOOP_END_PARAM_INDEX] - TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
					}
				}

				outResult.MapSampleIndex(sample.GetIndex(), currentSampleIndex);
				outResult.AddSample(E4SampleResult(sample.GetName(), std::move(convertedSampleData), sample.GetSampleRate(), GetSampleChannels(sample), 
					canLoop, (sampleFormat & E4SampleVariables::SAMPLE_RELEASE_FLAG) == E4SampleVariables::SAMPLE_RELEASE_FLAG, loopStart, loopEnd));

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

		constexpr auto EMST_READ_OFFSET = 8ull;

		E4EMSt emst;
		reader.readTypeAtLocation(&emst, lastLoc + EMST_READ_OFFSET, EMST_DATA_READ_SIZE);

		outResult.SetCurrentPreset(emst.GetCurrentPreset());
	}

	return true;
}

// TODO: remove
bool E4BFunctions::IsAccountingForCords(const E4Result& result)
{
	bool isAccounting(true);
	for(const auto& preset : result.GetPresets())
	{
		int voiceIndex(0);
		for(const auto& voice : preset.GetVoices())
		{
			for(const auto& cord : voice.GetCords())
			{
				if(cord.GetSource() == LFO1_POLARITY_CENTER && cord.GetDest() == AMP_VOLUME) { continue; }
				if(cord.GetSource() == LFO1_POLARITY_CENTER && cord.GetDest() == PITCH) { continue; }
				if(cord.GetSource() == LFO1_POLARITY_CENTER && cord.GetDest() == FILTER_FREQ) { continue; }
				if(cord.GetSource() == LFO1_POLARITY_CENTER && cord.GetDest() == AMP_PAN) { continue; }
				if(cord.GetSource() == FILTER_ENV_POLARITY_POS && cord.GetDest() == FILTER_FREQ) { continue; }
				if(cord.GetSource() == PITCH_WHEEL && cord.GetDest() == PITCH) { continue; }
				if(cord.GetSource() == MIDI_A && cord.GetDest() == AMP_VOLUME) { continue; }
				if(cord.GetSource() == VEL_POLARITY_POS && cord.GetDest() == FILTER_RES) { continue; }
				if(cord.GetSource() == VEL_POLARITY_LESS && cord.GetDest() == AMP_VOLUME) { continue; }
				if(cord.GetSource() == VEL_POLARITY_LESS && cord.GetDest() == FILTER_ENV_ATTACK) { continue; }
				if(cord.GetSource() == VEL_POLARITY_LESS && cord.GetDest() == FILTER_FREQ) { continue; }
				if(cord.GetSource() == VEL_POLARITY_CENTER && cord.GetDest() == AMP_PAN) { continue; }
				if(cord.GetSource() == KEY_POLARITY_CENTER && cord.GetDest() == FILTER_FREQ) { continue; }
				if(cord.GetSource() == MOD_WHEEL && cord.GetDest() == FILTER_FREQ) { continue; }
				if(cord.GetSource() == MOD_WHEEL && cord.GetDest() == CORD_3_AMT) { continue; }
				if(cord.GetSource() == PRESSURE && cord.GetDest() == AMP_ENV_ATTACK) { continue; }
				if(cord.GetSource() == PEDAL && cord.GetDest() == AMP_VOLUME) { continue; }

				// Skipping these:
				if(cord.GetSource() == 0ui8 && cord.GetDest() == CORD_3_AMT) { continue; } // This means controlling vibrato is OFF.
				if(cord.GetSource() == 0ui8 && cord.GetDest() == PITCH) { continue; } // This means the pitch wheel is OFF.
				if(cord.GetSource() == FOOTSWITCH_1 && cord.GetDest() == KEY_SUSTAIN) { continue; } // Emax II specific
				if(cord.GetSource() == 0ui8 && cord.GetDest() == 0ui8) { continue; }

				Logger::LogMessage("(preset: %s, voice: %d) Cord was not accounted: src: %d, dst: %d", preset.GetName().c_str(), voiceIndex, cord.GetSource(), cord.GetDest());
				isAccounting = false;
			}

			++voiceIndex;
		}
	}

	return isAccounting;
}