#include "Header/E4B/Helpers/E4BHelpers.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include <algorithm>
#include <cassert>

E4ZoneNoteData E4BHelpers::GetE4ZoneNoteFromBankNoteRange(const BankNoteRange& noteRange)
{
    return E4ZoneNoteData(noteRange.m_low, noteRange.m_high);
}

E4Envelope E4BHelpers::GetE4EnvFromADSREnv(const ADSR_Envelope& adsrEnv)
{
    return E4Envelope(adsrEnv.m_attackSec, adsrEnv.m_decaySec, adsrEnv.m_holdSec,
        adsrEnv.m_releaseSec, adsrEnv.m_delaySec, adsrEnv.m_sustainDB);
}

E4LFO E4BHelpers::GetE4LFOFromBankLFO(const BankLFO& lfo)
{
    return E4LFO(lfo.m_rate, lfo.m_shape, lfo.m_delay, lfo.m_keySync);
}

EEOSCordSource E4BHelpers::GetE4CordSrcFromRTControlSrc(const ERealtimeControlSrc src)
{
    switch(src)
    {
        default:
        {
            // Shouldn't ever hit this..
            assert(false);
            return EEOSCordSource::SRC_OFF;
        }

        case ERealtimeControlSrc::SRC_OFF: { return EEOSCordSource::SRC_OFF; }
        case ERealtimeControlSrc::MIDI_A: { return EEOSCordSource::MIDI_A; }
        case ERealtimeControlSrc::MIDI_B: { return EEOSCordSource::MIDI_B; }
        case ERealtimeControlSrc::PEDAL: { return EEOSCordSource::PEDAL; }
        case ERealtimeControlSrc::PRESSURE: { return EEOSCordSource::PRESSURE; }
        case ERealtimeControlSrc::MOD_WHEEL: { return EEOSCordSource::MOD_WHEEL; }
        case ERealtimeControlSrc::FOOTSWITCH_1: { return EEOSCordSource::FOOTSWITCH_1; }
        case ERealtimeControlSrc::PITCH_WHEEL: { return EEOSCordSource::PITCH_WHEEL; }
        case ERealtimeControlSrc::KEY_POLARITY_POS: { return EEOSCordSource::KEY_POLARITY_POS; }
        case ERealtimeControlSrc::KEY_POLARITY_CENTER: { return EEOSCordSource::KEY_POLARITY_CENTER; }
        case ERealtimeControlSrc::VEL_POLARITY_POS: { return EEOSCordSource::VEL_POLARITY_POS; }
        case ERealtimeControlSrc::VEL_POLARITY_LESS: { return EEOSCordSource::VEL_POLARITY_LESS; }
        case ERealtimeControlSrc::VEL_POLARITY_CENTER: { return EEOSCordSource::VEL_POLARITY_CENTER; }
        case ERealtimeControlSrc::LFO1_POLARITY_CENTER: { return EEOSCordSource::LFO1_POLARITY_CENTER; }
        case ERealtimeControlSrc::FILTER_ENV_POLARITY_POS: { return EEOSCordSource::FILTER_ENV_POLARITY_POS; }
    }
}

EEOSCordDest E4BHelpers::GetE4CordDstFromRTControlDst(const ERealtimeControlDst dst)
{
    switch(dst)
    {
        default:
        {
            // Shouldn't ever hit this..
            assert(false);
            return EEOSCordDest::DST_OFF;
        }

        case ERealtimeControlDst::DST_OFF: { return EEOSCordDest::DST_OFF; }
        case ERealtimeControlDst::PITCH: { return EEOSCordDest::PITCH; }
        case ERealtimeControlDst::AMP_PAN: { return EEOSCordDest::AMP_PAN; }
        case ERealtimeControlDst::AMP_VOLUME: { return EEOSCordDest::AMP_VOLUME; }
        case ERealtimeControlDst::AMP_ENV_ATTACK: { return EEOSCordDest::AMP_ENV_ATTACK; }
        case ERealtimeControlDst::FILTER_ENV_ATTACK: { return EEOSCordDest::FILTER_ENV_ATTACK; }
        case ERealtimeControlDst::FILTER_FREQ: { return EEOSCordDest::FILTER_FREQ; }
        case ERealtimeControlDst::FILTER_RES: { return EEOSCordDest::FILTER_RES; }
        case ERealtimeControlDst::VIBRATO: { return EEOSCordDest::CORD_3_AMT; }
        case ERealtimeControlDst::KEY_SUSTAIN: { return EEOSCordDest::KEY_SUSTAIN; }
    }
}

E4Cord E4BHelpers::GetE4CordFromBankRTControl(const BankRealtimeControl& control)
{
    return E4Cord(GetE4CordSrcFromRTControlSrc(control.m_src),
        GetE4CordDstFromRTControlDst(control.m_dst),
        E4VoiceHelpers::ConvertPercentToByteF(control.m_amount));
}

std::vector<E4Voice> E4BHelpers::GetE4VoicesFromBankVoices(const std::vector<BankVoice>& voices)
{
    std::vector<E4Voice> outVoices{};
    for(const auto& voice : voices) { outVoices.emplace_back(voice); }
    return outVoices;
}

std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> E4BHelpers::ConvertToE4Name(const std::string_view& name)
{
    std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> outName{};
    const uint32_t nameClamped(std::clamp(static_cast<uint32_t>(name.length()), 0u, E4BVariables::EOS_E4_MAX_NAME_LEN));
    for(auto i(0u); i < nameClamped; ++i) { outName[i] = name[i]; }
    return outName;
}

std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> E4BHelpers::ConvertToE4ChunkName(const std::string_view& name)
{
    std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> outName{};
    const uint64_t nameClamped(std::clamp(name.length(), 0ull, E4BVariables::EOS_CHUNK_NAME_LEN));
    for(auto i(0u); i < nameClamped; ++i) { outName[i] = name[i]; }
    return outName;
}