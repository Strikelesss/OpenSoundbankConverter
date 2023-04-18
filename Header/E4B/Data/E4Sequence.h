#pragma once
#include "Header/E4B/Helpers/E4BVariables.h"
#include <vector>

struct ReadLocationHandle;
struct BinaryWriter;
struct BinaryReader;
struct E4TOCChunk;

constexpr auto SEQUENCE_DATA_READ_SIZE = 18ull;

struct E4Sequence final
{
    explicit E4Sequence(const E4TOCChunk& chunk, BinaryReader& reader);

    void write(BinaryWriter& writer) const;

    [[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_seqIndex); }
    [[nodiscard]] std::string_view GetName() const { return {m_name.data(), m_name.size()}; }
    [[nodiscard]] std::vector<char>& GetData() { return m_midiData; }
    
protected:
    void readAtLocation(ReadLocationHandle& readHandle);
    
    /*
     * Read data (follows SEQUENCE_DATA_READ_SIZE)
     */
    
    uint16_t m_seqIndex = 0ui16; // requires byteswap
    std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};

    /*
     * Allocated data
     */
    std::vector<char> m_midiData{};
};