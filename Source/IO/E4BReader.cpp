#include "Header/IO/E4BReader.h"
#include "Header/IO/BinaryReader.h"
#include "Header/Logger.h"
#include "Header/MathFunctions.h"
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Data/E4Preset.h"
#include "Header/E4B/Data/E3Sample.h"
#include "Header/E4B/Data/E4Sequence.h"
#include "Header/E4B/Data/EMSt.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/BinaryWriter.h"

E4TOCChunk::E4TOCChunk(std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN>&& name, const uint32_t length, const uint32_t startOffset)
    : m_chunkName(std::move(name)), m_chunkLength(MathFunctions::byteswapUINT32(length)), m_chunkStartOffset(MathFunctions::byteswapUINT32(startOffset)) {}

void E4TOCChunk::write(BinaryWriter& writer) const
{
    writer.writeType(m_chunkName.data(), sizeof(char) * m_chunkName.size());
    writer.writeType(&m_chunkLength);
    writer.writeType(&m_chunkStartOffset);
}

void E4TOCChunk::read(BinaryReader& reader)
{
    reader.readType(m_chunkName.data(), sizeof(char) * E4BVariables::EOS_CHUNK_NAME_LEN);
    reader.readType(&m_chunkLength);
    reader.readType(&m_chunkStartOffset);
}

uint32_t E4TOCChunk::GetLength() const
{
    return MathFunctions::byteswapUINT32(m_chunkLength);
}

uint32_t E4TOCChunk::GetStartOffset() const
{
    return MathFunctions::byteswapUINT32(m_chunkStartOffset);
}

E4DataChunk::E4DataChunk(std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN>&& name, const uint32_t length)
    : m_chunkName(std::move(name)), m_chunkLength(MathFunctions::byteswapUINT32(length)) {}

void E4DataChunk::write(BinaryWriter& writer) const
{
    writer.writeType(m_chunkName.data(), sizeof(char) * m_chunkName.size());
    writer.writeType(&m_chunkLength);
}

void E4DataChunk::read(BinaryReader& reader)
{
    reader.readType(m_chunkName.data(), sizeof(char) * E4BVariables::EOS_CHUNK_NAME_LEN);
    reader.readType(&m_chunkLength);
}

void E4DataChunk::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(m_chunkName.data(), sizeof(char) * E4BVariables::EOS_CHUNK_NAME_LEN);
    readHandle.readType(&m_chunkLength);
}

uint32_t E4DataChunk::GetLength() const
{
    return MathFunctions::byteswapUINT32(m_chunkLength);
}

Soundbank E4BReader::ProcessFile(const std::filesystem::path& file)
{
    Soundbank outResult(file.filename().replace_extension("").string());
    
    BinaryReader reader;
    if(reader.readFile(file))
    {
        E4DataChunk FORMChunk;
        FORMChunk.read(reader);

        if (std::strncmp(FORMChunk.GetName().data(), E4BVariables::EOS_FORM_TAG.data(), E4BVariables::EOS_FORM_TAG.length()) != 0) { return outResult; }

        if(FORMChunk.GetLength() > 0u)
        {
            std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> E4B0Chunk{};
            reader.readType(E4B0Chunk.data(), sizeof(char) * E4BVariables::EOS_CHUNK_NAME_LEN);

            if (std::strncmp(E4B0Chunk.data(), E4BVariables::EOS_E4_FORMAT_TAG.data(), E4BVariables::EOS_E4_FORMAT_TAG.length()) != 0) { return outResult; }

            E4DataChunk TOC1Chunk;
            TOC1Chunk.read(reader);

            if (std::strncmp(TOC1Chunk.GetName().data(), E4BVariables::EOS_TOC_TAG.data(), E4BVariables::EOS_TOC_TAG.length()) != 0) { return outResult; }

            if(TOC1Chunk.GetLength() > 0u)
            {
                uint32_t tocChunkLengths(0u);
                
                const uint64_t numTOCChunks(TOC1Chunk.GetLength() / E4BVariables::EOS_CHUNK_TOTAL_LEN);
                for(uint32_t i(0u); i < numTOCChunks; ++i)
                {
                    E4TOCChunk currentChunk;
                    currentChunk.read(reader);

                    if (std::strncmp(currentChunk.GetName().data(), E4BVariables::EOS_E4_PRESET_TAG.data(), E4BVariables::EOS_E4_PRESET_TAG.length()) == 0)
                    {
                        E4Preset preset(currentChunk, reader);

                        std::vector<BankVoice> voices;
                        for(const auto& voice : preset.GetVoices())
                        {
                            VerifyRealtimeCordsAccounted(preset, voice, voices.size() - 1);
                            for(const auto& zone : voice.GetZones())
                            {
                                voices.emplace_back(GetBankVoiceFromE4Zone(voice, zone));   
                            }
                        }
                        
                        outResult.m_presets.emplace_back(preset.GetIndex(), std::string(preset.GetName()), std::move(voices));

                        // Finished reading, skip rest of TOC chunk.
                        reader.skipBytes(E4BVariables::EOS_CHUNK_TOTAL_LEN - sizeof(E4TOCChunk));

                        tocChunkLengths += currentChunk.GetLength() + sizeof(uint16_t);
                    }
                    else
                    {
                        if (std::strncmp(currentChunk.GetName().data(), E4BVariables::EOS_E3_SAMPLE_TAG.data(), E4BVariables::EOS_E3_SAMPLE_TAG.length()) == 0)
                        {
                            E3Sample sample(currentChunk, reader);

                            auto& sampleData(sample.GetData());
                            outResult.m_samples.emplace_back(sample.GetIndex(), std::string(sample.GetName()), std::move(sampleData),
                                sample.GetSampleRate(), sample.GetNumChannels(), sample.IsLooping(), sample.IsLoopReleasing(), sample.GetLoopStart(),
                                sample.GetLoopEnd());
                            
                            // Finished reading, skip rest of TOC chunk.
                            reader.skipBytes(E4BVariables::EOS_CHUNK_TOTAL_LEN - sizeof(E4TOCChunk));

                            tocChunkLengths += currentChunk.GetLength() + sizeof(uint16_t);
                        }
                        else
                        {
                            if (std::strncmp(currentChunk.GetName().data(), E4BVariables::EOS_E4Ma_TAG.data(), E4BVariables::EOS_E4Ma_TAG.length()) == 0)
                            {
                                // Skip E4Ma
                                reader.skipBytes(E4BVariables::EOS_CHUNK_TOTAL_LEN - sizeof(E4TOCChunk));

                                tocChunkLengths += currentChunk.GetLength() + sizeof(uint16_t);
                            }
                            else
                            {
                                if (std::strncmp(currentChunk.GetName().data(), E4BVariables::EOS_E4_SEQ_TAG.data(), E4BVariables::EOS_E4_SEQ_TAG.length()) == 0)
                                {
                                    E4Sequence sequence(currentChunk, reader);

                                    auto& midiData(sequence.GetData());
                                    outResult.m_sequences.emplace_back(sequence.GetIndex(), std::string(sequence.GetName()), std::move(midiData));
                                    
                                    // Finished reading, skip rest of TOC chunk.
                                    reader.skipBytes(E4BVariables::EOS_CHUNK_TOTAL_LEN - sizeof(E4TOCChunk));
                                    
                                    tocChunkLengths += currentChunk.GetLength() + sizeof(uint16_t);
                                }
                                else
                                {
                                    // Not supposed to reach here.
                                    assert(false);
                                    return outResult;
                                }
                            }
                        }
                    }
                }

                assert(tocChunkLengths > 0u);
                if(tocChunkLengths > 0u)
                {
                    const uint64_t endLocation(reader.GetReadLocation() + tocChunkLengths + (numTOCChunks * sizeof(E4DataChunk)));

                    // Ensure that we even have an end chunk (some banks do not)
					if (endLocation != reader.GetData().size())
					{
						ReadLocationHandle readHandle(reader, endLocation);

						E4DataChunk EMStChunk;
						EMStChunk.readAtLocation(readHandle);

						if (std::strncmp(EMStChunk.GetName().data(), E4BVariables::EOS_EMSt_TAG.data(), E4BVariables::EOS_EMSt_TAG.length()) == 0)
						{
							E4EMSt emst;
							emst.readAtLocation(readHandle);

							outResult.m_defaultPreset = emst.GetCurrentPreset();
						}
						else
						{
							// End location should be an EMSt, this is not valid.
							assert(false);
						}
					}
                }
            }
        }
    }

    return outResult;
}

BankVoice E4BReader::GetBankVoiceFromE4Zone(const E4Voice& e4Voice, const E4Zone& e4Zone)
{
    BankVoice outVoice;
    outVoice.m_volume = e4Voice.GetVolume();
    outVoice.m_pan = e4Voice.GetPan();
    outVoice.m_chorusAmount = e4Voice.GetChorusAmount();
    outVoice.m_chorusWidth = e4Voice.GetChorusWidth();
    outVoice.m_fineTune = e4Voice.GetFineTune();
    outVoice.m_filterFrequency = e4Voice.GetFilterFrequency();
    outVoice.m_filterQ = e4Voice.GetFilterQ();
    outVoice.m_coarseTune = e4Voice.GetCoarseTune();
    outVoice.m_transpose = e4Voice.GetTranspose();

    // For the key range, if there's only 1 zone the defined key range in the zone is empty, and the voice zone will be used.
    const auto& keyRange(e4Voice.GetZones().size() > 1 ? e4Zone.GetKeyZoneRange() : e4Voice.GetKeyZoneRange());
    outVoice.m_keyZone = BankNoteRange(keyRange.GetLow(), keyRange.GetHigh());

    const auto& velRange(e4Zone.GetVelocityRange());
    outVoice.m_velocityZone = BankNoteRange(velRange.GetLow(), velRange.GetHigh());
    
    outVoice.m_sampleIndex = e4Zone.GetSampleIndex();
    outVoice.m_originalKey = e4Zone.GetOriginalKey();

    outVoice.m_ampEnv = GetADSREnvelopeFromE4Envelope(e4Voice.GetAmpEnv());
    outVoice.m_ampEnv.m_delaySec = e4Voice.GetKeyDelay();
    
    outVoice.m_filterEnv = GetADSREnvelopeFromE4Envelope(e4Voice.GetFilterEnv());

    outVoice.m_lfo1 = GetBankLFOFromE4LFO(e4Voice.GetLFO1());

    outVoice.m_realtimeControls = GetBankRTControlsFromE4Voice(e4Voice);
    
    return outVoice;
}

ADSR_Envelope E4BReader::GetADSREnvelopeFromE4Envelope(const E4Envelope& e4Envelope)
{
    const auto attackSec(e4Envelope.GetAttack2Sec());
    const auto decaySec(e4Envelope.GetDecay2Sec());
    const auto delaySec(e4Envelope.GetAttack1Sec());
    const auto holdSec(e4Envelope.GetDecay1Sec());
    const auto sustainDB(e4Envelope.GetDecay2Level());
    const auto releaseSec(e4Envelope.GetRelease1Sec());
    
    return ADSR_Envelope(attackSec, decaySec, holdSec, sustainDB, releaseSec, delaySec);
}

BankLFO E4BReader::GetBankLFOFromE4LFO(const E4LFO& e4LFO)
{
    return BankLFO(e4LFO.GetRate(), e4LFO.GetShape(), e4LFO.GetDelay(), e4LFO.IsKeySync());
}

BankRealtimeControl E4BReader::GetBankRTControlsFromE4Cord(const E4Cord& cord)
{
    return BankRealtimeControl(GetBankRTControlSrcFromE4CordSrc(cord.GetSource()),
        GetBankRTControlDstFromE4CordDst(cord.GetDest()), E4VoiceHelpers::ConvertByteToPercentF(cord.GetAmount()));
}

ERealtimeControlSrc E4BReader::GetBankRTControlSrcFromE4CordSrc(const EEOSCordSource src)
{
    switch(src)
    {
        default:
        {
            // Shouldn't ever hit this..
            //assert(false);
            Logger::LogMessage("Cord source was unaccounted for!");
            return ERealtimeControlSrc::SRC_OFF;
        }

        case EEOSCordSource::SRC_OFF: { return ERealtimeControlSrc::SRC_OFF; }
        case EEOSCordSource::MIDI_A: { return ERealtimeControlSrc::MIDI_A; }
        case EEOSCordSource::MIDI_B: { return ERealtimeControlSrc::MIDI_B; }
        case EEOSCordSource::PEDAL: { return ERealtimeControlSrc::PEDAL; }
        case EEOSCordSource::PRESSURE: { return ERealtimeControlSrc::PRESSURE; }
        case EEOSCordSource::MOD_WHEEL: { return ERealtimeControlSrc::MOD_WHEEL; }
        case EEOSCordSource::FOOTSWITCH_1: { return ERealtimeControlSrc::FOOTSWITCH_1; }
        case EEOSCordSource::PITCH_WHEEL: { return ERealtimeControlSrc::PITCH_WHEEL; }
        case EEOSCordSource::KEY_POLARITY_POS: { return ERealtimeControlSrc::KEY_POLARITY_POS; }
        case EEOSCordSource::KEY_POLARITY_CENTER: { return ERealtimeControlSrc::KEY_POLARITY_CENTER; }
        case EEOSCordSource::VEL_POLARITY_POS: { return ERealtimeControlSrc::VEL_POLARITY_POS; }
        case EEOSCordSource::VEL_POLARITY_LESS: { return ERealtimeControlSrc::VEL_POLARITY_LESS; }
        case EEOSCordSource::VEL_POLARITY_CENTER: { return ERealtimeControlSrc::VEL_POLARITY_CENTER; }
        case EEOSCordSource::LFO1_POLARITY_CENTER: { return ERealtimeControlSrc::LFO1_POLARITY_CENTER; }
        case EEOSCordSource::FILTER_ENV_POLARITY_POS: { return ERealtimeControlSrc::FILTER_ENV_POLARITY_POS; }
    }
}

ERealtimeControlDst E4BReader::GetBankRTControlDstFromE4CordDst(const EEOSCordDest dst)
{
    switch(dst)
    {
        default:
        {
            // Shouldn't ever hit this..
            //assert(false);
            Logger::LogMessage("Cord destination was unaccounted for!");
            return ERealtimeControlDst::DST_OFF;
        }

        case EEOSCordDest::DST_OFF: { return ERealtimeControlDst::DST_OFF; }
        case EEOSCordDest::PITCH: { return ERealtimeControlDst::PITCH; }
        case EEOSCordDest::AMP_PAN: { return ERealtimeControlDst::AMP_PAN; }
        case EEOSCordDest::AMP_VOLUME: { return ERealtimeControlDst::AMP_VOLUME; }
        case EEOSCordDest::AMP_ENV_ATTACK: { return ERealtimeControlDst::AMP_ENV_ATTACK; }
        case EEOSCordDest::FILTER_ENV_ATTACK: { return ERealtimeControlDst::FILTER_ENV_ATTACK; }
        case EEOSCordDest::FILTER_FREQ: { return ERealtimeControlDst::FILTER_FREQ; }
        case EEOSCordDest::FILTER_RES: { return ERealtimeControlDst::FILTER_RES; }
        case EEOSCordDest::CORD_3_AMT: { return ERealtimeControlDst::VIBRATO; }
        case EEOSCordDest::KEY_SUSTAIN: { return ERealtimeControlDst::KEY_SUSTAIN; }
    }
}

std::array<BankRealtimeControl, MAX_REALTIME_CONTROLS> E4BReader::GetBankRTControlsFromE4Voice(const E4Voice& voice)
{
    std::array<BankRealtimeControl, MAX_REALTIME_CONTROLS> outRTControls{};

    size_t index(0);
    for(const auto& cord : voice.GetCords())
    {
        outRTControls[index] = GetBankRTControlsFromE4Cord(cord);
        ++index;
    }
    
    return outRTControls;
}

void E4BReader::VerifyRealtimeCordsAccounted(const E4Preset& e4Preset, const E4Voice& e4Voice, const uint64_t voiceIndex)
{
    for (const auto& cord : e4Voice.GetCords())
    {
        if (cord.GetSource() == EEOSCordSource::LFO1_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::AMP_VOLUME) { continue; }
        if (cord.GetSource() == EEOSCordSource::LFO1_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::PITCH) { continue; }
        if (cord.GetSource() == EEOSCordSource::LFO1_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::FILTER_FREQ) { continue; }
        if (cord.GetSource() == EEOSCordSource::LFO1_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::AMP_PAN) { continue; }
        if (cord.GetSource() == EEOSCordSource::FILTER_ENV_POLARITY_POS && cord.GetDest() == EEOSCordDest::FILTER_FREQ) { continue; }
        if (cord.GetSource() == EEOSCordSource::PITCH_WHEEL && cord.GetDest() == EEOSCordDest::PITCH) { continue; }
        if (cord.GetSource() == EEOSCordSource::MIDI_A && cord.GetDest() == EEOSCordDest::AMP_VOLUME) { continue; }
        if (cord.GetSource() == EEOSCordSource::VEL_POLARITY_POS && cord.GetDest() == EEOSCordDest::FILTER_RES) { continue; }
        if (cord.GetSource() == EEOSCordSource::VEL_POLARITY_LESS && cord.GetDest() == EEOSCordDest::AMP_VOLUME) { continue; }
        if (cord.GetSource() == EEOSCordSource::VEL_POLARITY_LESS && cord.GetDest() == EEOSCordDest::FILTER_ENV_ATTACK) { continue; }
        if (cord.GetSource() == EEOSCordSource::VEL_POLARITY_LESS && cord.GetDest() == EEOSCordDest::FILTER_FREQ) { continue; }
        if (cord.GetSource() == EEOSCordSource::VEL_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::AMP_PAN) { continue; }
        if (cord.GetSource() == EEOSCordSource::KEY_POLARITY_CENTER && cord.GetDest() == EEOSCordDest::FILTER_FREQ) { continue; }
        if (cord.GetSource() == EEOSCordSource::MOD_WHEEL && cord.GetDest() == EEOSCordDest::FILTER_FREQ) { continue; }
        if (cord.GetSource() == EEOSCordSource::MOD_WHEEL && cord.GetDest() == EEOSCordDest::CORD_3_AMT) { continue; }
        if (cord.GetSource() == EEOSCordSource::PRESSURE && cord.GetDest() == EEOSCordDest::AMP_ENV_ATTACK) { continue; }
        if (cord.GetSource() == EEOSCordSource::PEDAL && cord.GetDest() == EEOSCordDest::AMP_VOLUME) { continue; }

        // Skipping these:
        if (cord.GetSource() == EEOSCordSource::SRC_OFF && cord.GetDest() == EEOSCordDest::CORD_3_AMT) { continue; } // This means controlling vibrato is OFF.
        if (cord.GetSource() == EEOSCordSource::SRC_OFF && cord.GetDest() == EEOSCordDest::PITCH) { continue; } // This means the pitch wheel is OFF.
        if (cord.GetSource() == EEOSCordSource::FOOTSWITCH_1 && cord.GetDest() == EEOSCordDest::KEY_SUSTAIN) { continue; } // Emax II specific
        if (cord.GetSource() == EEOSCordSource::SRC_OFF && cord.GetDest() == EEOSCordDest::DST_OFF) { continue; }

        Logger::LogMessage("(preset: %s, voice: %d) Cord was not accounted: src: %d, dst: %d", e4Preset.GetName().data(),
            voiceIndex, cord.GetSource(), cord.GetDest());
    }
}
