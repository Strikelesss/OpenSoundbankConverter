#include "Header/E4B/Data/E4Voice.h"
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Helpers/E4BHelpers.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/IO/E4BReader.h"

E4Voice::E4Voice(const E4TOCChunk& chunk, const uint16_t presetDataSize, const uint16_t voiceOffset, BinaryReader& reader)
{
    const auto voicePos(voiceOffset + chunk.GetStartOffset() + presetDataSize + E4BVariables::EOS_CHUNK_NAME_OFFSET);
    ReadLocationHandle readHandle(reader, voicePos);
    readAtLocation(readHandle);
    
    for (uint16_t i(0ui16); i < m_zoneCount; ++i)
    {
        E4Zone zone;
        zone.readAtLocation(readHandle);

        m_zones.emplace_back(std::move(zone));
    }
}

E4Voice::E4Voice(const BankVoice& voice) : m_totalVoiceSize(_byteswap_ushort(VOICE_1_ZONE_DATA_SIZE)),
    m_keyData(E4BHelpers::GetE4ZoneNoteFromBankNoteRange(voice.m_keyZone)), m_velData(E4BHelpers::GetE4ZoneNoteFromBankNoteRange(voice.m_velocityZone)),
    m_keyDelay(_byteswap_ushort(static_cast<uint16_t>(voice.m_ampEnv.m_delaySec * 1000.))), m_transpose(voice.m_transpose), m_coarseTune(voice.m_coarseTune),
    m_fineTune(E4VoiceHelpers::ConvertFineTuneToByte(voice.m_fineTune)), m_chorusWidth(E4VoiceHelpers::ConvertChorusWidthToByte(voice.m_chorusWidth)),
    m_chorusAmount(E4VoiceHelpers::ConvertPercentToByteF(voice.m_chorusAmount)), m_volume(voice.m_volume), m_pan(voice.m_pan),
    m_filterFrequency(E4VoiceHelpers::ConvertFilterFrequencyToByte(voice.m_filterFrequency)), m_filterQ(E4VoiceHelpers::ConvertPercentToByteF(voice.m_filterQ)),
    m_ampEnv(E4BHelpers::GetE4EnvFromADSREnv(voice.m_ampEnv)), m_filterEnv(E4BHelpers::GetE4EnvFromADSREnv(voice.m_filterEnv)), m_lfo1(E4BHelpers::GetE4LFOFromBankLFO(voice.m_lfo1))
{
    PopulateCordsFromBankVoice(voice);
    
    // Add only 1 zone:
    m_zones.emplace_back(static_cast<uint16_t>(voice.m_sampleIndex + 1ui16), voice.m_originalKey);

    // If the filter frequency isn't 20,000, pick 4 Pole Lowpass, if it is, pick No Filter if there's no filter resonance or realtime filter modifications:

    const bool hasFilter(m_filterFrequency < 255ui8 || m_filterQ > 0ui8);
    if(!hasFilter)
    {
        for(const auto& cord : m_cords)
        {
            const auto dest(cord.GetDest());
            if((dest == EEOSCordDest::FILTER_FREQ || dest == EEOSCordDest::FILTER_RES) && cord.GetAmount() != 0ui8)
            {
                m_filterType = EEOSFilterType::FOUR_POLE_LOWPASS;
                break;
            }
        }
    }
    else
    {
        m_filterType = EEOSFilterType::FOUR_POLE_LOWPASS;
    }
}

void E4Voice::write(BinaryWriter& writer) const
{
    writer.writeType(&m_totalVoiceSize);
    writer.writeType(&m_zoneCount);
    writer.writeType(&m_group);
    writer.writeType(m_amplifierData.data(), sizeof(int8_t) * m_amplifierData.size());
    m_keyData.write(writer);
    m_velData.write(writer);
    m_rtData.write(writer);
    writer.writeType(&m_possibleRedundant1);
    writer.writeType(&m_keyAssignGroup);
    writer.writeType(&m_keyDelay);
    writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    writer.writeType(&m_sampleOffset);
    writer.writeType(&m_transpose);
    writer.writeType(&m_coarseTune);
    writer.writeType(&m_fineTune);
    writer.writeType(&m_glideRate);
    writer.writeType(&m_fixedPitch);
    writer.writeType(&m_keyMode);
    writer.writeType(&m_possibleRedundant3);
    writer.writeType(&m_chorusWidth);
    writer.writeType(&m_chorusAmount);
    writer.writeType(m_possibleRedundant4.data(), sizeof(int8_t) * m_possibleRedundant4.size());
    writer.writeType(&m_keyLatch);
    writer.writeType(m_possibleRedundant5.data(), sizeof(int8_t) * m_possibleRedundant5.size());
    writer.writeType(&m_glideCurve);
    writer.writeType(&m_volume);
    writer.writeType(&m_pan);
    writer.writeType(&m_possibleRedundant6);
    writer.writeType(&m_ampEnvDynRange);

    const uint8_t filterType(static_cast<uint8_t>(m_filterType));
    writer.writeType(&filterType);
    
    writer.writeType(&m_possibleRedundant7);
    writer.writeType(&m_filterFrequency);
    writer.writeType(&m_filterQ);
    writer.writeType(m_possibleRedundant8.data(), sizeof(int8_t) * m_possibleRedundant8.size());
    m_ampEnv.write(writer);
    writer.writeType(m_possibleRedundant9.data(), sizeof(int8_t) * m_possibleRedundant9.size());
    m_filterEnv.write(writer);
    writer.writeType(m_possibleRedundant10.data(), sizeof(int8_t) * m_possibleRedundant10.size());
    m_auxEnv.write(writer);
    writer.writeType(m_possibleRedundant11.data(), sizeof(int8_t) * m_possibleRedundant11.size());
    m_lfo1.write(writer);
    m_lfo2.write(writer);
    writer.writeType(m_possibleRedundant12.data(), sizeof(int8_t) * m_possibleRedundant12.size());

    for(const auto& cord : m_cords)
    {
        cord.write(writer);
    }

    // Only write 1 zone:
    assert(!m_zones.empty());
    if(!m_zones.empty())
    {
        m_zones[0].write(writer);
    }
}

void E4Voice::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_totalVoiceSize);
    readHandle.readType(&m_zoneCount);
    readHandle.readType(&m_group);
    readHandle.readType(m_amplifierData.data(), sizeof(int8_t) * m_amplifierData.size());
    m_keyData.readAtLocation(readHandle);
    m_velData.readAtLocation(readHandle);
    m_rtData.readAtLocation(readHandle);
    readHandle.readType(&m_possibleRedundant1);
    readHandle.readType(&m_keyAssignGroup);
    readHandle.readType(&m_keyDelay);
    readHandle.readType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    readHandle.readType(&m_sampleOffset);
    readHandle.readType(&m_transpose);
    readHandle.readType(&m_coarseTune);
    readHandle.readType(&m_fineTune);
    readHandle.readType(&m_glideRate);
    readHandle.readType(&m_fixedPitch);
    readHandle.readType(&m_keyMode);
    readHandle.readType(&m_possibleRedundant3);
    readHandle.readType(&m_chorusWidth);
    readHandle.readType(&m_chorusAmount);
    readHandle.readType(m_possibleRedundant4.data(), sizeof(int8_t) * m_possibleRedundant4.size());
    readHandle.readType(&m_keyLatch);
    readHandle.readType(m_possibleRedundant5.data(), sizeof(int8_t) * m_possibleRedundant5.size());
    readHandle.readType(&m_glideCurve);
    readHandle.readType(&m_volume);
    readHandle.readType(&m_pan);
    readHandle.readType(&m_possibleRedundant6);
    readHandle.readType(&m_ampEnvDynRange);

    uint8_t filterType;
    readHandle.readType(&filterType);
    m_filterType = static_cast<EEOSFilterType>(filterType);
    
    readHandle.readType(&m_possibleRedundant7);
    readHandle.readType(&m_filterFrequency);
    readHandle.readType(&m_filterQ);
    readHandle.readType(m_possibleRedundant8.data(), sizeof(int8_t) * m_possibleRedundant8.size());
    m_ampEnv.readAtLocation(readHandle);
    readHandle.readType(m_possibleRedundant9.data(), sizeof(int8_t) * m_possibleRedundant9.size());
    m_filterEnv.readAtLocation(readHandle);
    readHandle.readType(m_possibleRedundant10.data(), sizeof(int8_t) * m_possibleRedundant10.size());
    m_auxEnv.readAtLocation(readHandle);
    readHandle.readType(m_possibleRedundant11.data(), sizeof(int8_t) * m_possibleRedundant11.size());
    m_lfo1.readAtLocation(readHandle);
    m_lfo2.readAtLocation(readHandle);
    readHandle.readType(m_possibleRedundant12.data(), sizeof(int8_t) * m_possibleRedundant12.size());

    for(auto& cord : m_cords)
    {
        cord.readAtLocation(readHandle);
    }
}

float E4Voice::GetChorusWidth() const
{
    return E4VoiceHelpers::GetChorusWidthPercent(m_chorusWidth);
}

float E4Voice::GetChorusAmount() const
{
    return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_chorusAmount), 2u);
}

uint16_t E4Voice::GetFilterFrequency() const
{
    return E4VoiceHelpers::ConvertByteToFilterFrequency(m_filterFrequency);
}

double E4Voice::GetFineTune() const
{
    return E4VoiceHelpers::ConvertByteToFineTune(m_fineTune);
}

float E4Voice::GetFilterQ() const
{
    return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_filterQ), 1u);
}

double E4Voice::GetKeyDelay() const
{
    return static_cast<double>(_byteswap_ushort(m_keyDelay)) / 1000.;
}

EEOSFilterType E4Voice::GetFilterType() const
{
    return m_filterType;
}

bool E4Voice::GetAmountFromCord(const EEOSCordSource src, const EEOSCordDest dst, float& outAmount) const
{
    for(const auto& cord : m_cords)
    {
        if(cord.GetSource() == src && cord.GetDest() == dst)
        {
            const auto amount(cord.GetAmount());
            if(amount != 0i8)
            {
                outAmount = std::roundf(E4VoiceHelpers::ConvertByteToPercentF(cord.GetAmount()));
                return true;
            }
        }
    }

    return false;
}

void E4Voice::ReplaceOrAddCord(const E4Cord& cord)
{
    // Replace if existing
    for(auto& existingCord : m_cords)
    {
        if(existingCord.GetSource() == cord.GetSource() && existingCord.GetDest() == cord.GetDest())
        {
            existingCord.SetAmount(cord.GetAmount());
            return;
        }
    }

    // Replace null cord
    for(auto& existingCord : m_cords)
    {
        if(existingCord.GetSource() == EEOSCordSource::SRC_OFF && existingCord.GetDest() == EEOSCordDest::DST_OFF)
        {
            existingCord = cord;
            break;
        }
    }
}

void E4Voice::DisableCord(const E4Cord& cord)
{
    for(auto& existingCord : m_cords)
    {
        if(existingCord.GetSource() == cord.GetSource() && existingCord.GetDest() == cord.GetDest())
        {
            existingCord.SetSrc(EEOSCordSource::SRC_OFF);
            break;
        }
    }
}

void E4Voice::PopulateCordsFromBankVoice(const BankVoice& voice)
{
    for(const auto& rtControl : voice.m_realtimeControls)
    {
        ReplaceOrAddCord(E4BHelpers::GetE4CordFromBankRTControl(rtControl));
    }

    float discard(0.f);

    // Disable default pitch wheel if unused
    if(!voice.GetAmountFromRTControl(ERealtimeControlSrc::PITCH_WHEEL, ERealtimeControlDst::PITCH, discard))
    {
        DisableCord(E4Cord(EEOSCordSource::PITCH_WHEEL, EEOSCordDest::PITCH, 0i8));
    }

    // Disable default mod wheel if unused
    if(!voice.GetAmountFromRTControl(ERealtimeControlSrc::MOD_WHEEL, ERealtimeControlDst::VIBRATO, discard))
    {
        DisableCord(E4Cord(EEOSCordSource::MOD_WHEEL, EEOSCordDest::CORD_3_AMT, 0i8));
    }
}