#pragma once
#include <filesystem>
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Helpers/E4BVariables.h"

enum struct EEOSCordDest : uint8_t;
enum struct EEOSCordSource : uint8_t;
struct E4Cord;
struct E4Preset;
struct E4LFO;
struct E4Envelope;
struct E4Zone;
struct E4Voice;
struct ReadLocationHandle;
struct BinaryReader;
struct BinaryWriter;

struct E4TOCChunk final
{
    E4TOCChunk() = default;
    explicit E4TOCChunk(std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN>&& name, uint32_t length, uint32_t startOffset);

    void write(BinaryWriter& writer) const;
    void read(BinaryReader& reader);
    
    [[nodiscard]] std::string_view GetName() const { return {m_chunkName.data(), m_chunkName.size()}; }
    [[nodiscard]] uint32_t GetLength() const;
    [[nodiscard]] uint32_t GetStartOffset() const;
    
protected:
    std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> m_chunkName{};
    uint32_t m_chunkLength = 0u; // requires byteswap
    uint32_t m_chunkStartOffset = 0u; // requires byteswap
};

struct E4DataChunk final
{
    E4DataChunk() = default;
    explicit E4DataChunk(std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN>&& name, uint32_t length);

    void write(BinaryWriter& writer) const;
    void read(BinaryReader& reader);
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] std::string_view GetName() const { return {m_chunkName.data(), m_chunkName.size()}; }
    [[nodiscard]] uint32_t GetLength() const;
    
protected:
    std::array<char, E4BVariables::EOS_CHUNK_NAME_LEN> m_chunkName{};
    uint32_t m_chunkLength = 0u; // requires byteswap
};

namespace E4BReader
{
    [[nodiscard]] Soundbank ProcessFile(const std::filesystem::path& file);
    [[nodiscard]] BankVoice GetBankVoiceFromE4Zone(const E4Voice& e4Voice, const E4Zone& e4Zone);
    [[nodiscard]] ADSR_Envelope GetADSREnvelopeFromE4Envelope(const E4Envelope& e4Envelope);
    [[nodiscard]] BankLFO GetBankLFOFromE4LFO(const E4LFO& e4LFO);
    [[nodiscard]] BankRealtimeControl GetBankRTControlsFromE4Cord(const E4Cord& cord);
    [[nodiscard]] ERealtimeControlSrc GetBankRTControlSrcFromE4CordSrc(EEOSCordSource src);
    [[nodiscard]] ERealtimeControlDst GetBankRTControlDstFromE4CordDst(EEOSCordDest dst);
    [[nodiscard]] std::array<BankRealtimeControl, MAX_REALTIME_CONTROLS> GetBankRTControlsFromE4Voice(const E4Voice& voice);
    void VerifyRealtimeCordsAccounted(const E4Preset& e4Preset, const E4Voice& e4Voice, uint64_t voiceIndex);
};