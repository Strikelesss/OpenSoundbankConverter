#pragma once
#include "E4BVariables.h"
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Data/E4Voice.h"

struct E4ZoneNoteData;

namespace E4BHelpers
{
    [[nodiscard]] E4ZoneNoteData GetE4ZoneNoteFromBankNoteRange(const BankNoteRange& noteRange);
    [[nodiscard]] E4Envelope GetE4EnvFromADSREnv(const ADSR_Envelope& adsrEnv);
    [[nodiscard]] E4LFO GetE4LFOFromBankLFO(const BankLFO& lfo);
    [[nodiscard]] EEOSCordSource GetE4CordSrcFromRTControlSrc(ERealtimeControlSrc src);
    [[nodiscard]] EEOSCordDest GetE4CordDstFromRTControlDst(ERealtimeControlDst dst);
    [[nodiscard]] E4Cord GetE4CordFromBankRTControl(const BankRealtimeControl& control);
    [[nodiscard]] std::vector<E4Voice> GetE4VoicesFromBankVoices(const std::vector<BankVoice>& voices);
    [[nodiscard]] std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> ConvertToE4Name(const std::string_view& name);
    [[nodiscard]] std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> ConvertToE4ChunkName(const std::string_view& name);
};