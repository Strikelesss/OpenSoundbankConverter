#pragma once
#include "E4Voice.h"
#include "Header/E4B/Helpers/E4BVariables.h"

struct BinaryWriter;
struct BankPreset;
struct BinaryReader;
struct E4TOCChunk;

constexpr size_t PRESET_DATA_READ_SIZE = 84; // This may be 82 instead, unsure if the voice data size is uint16 or uint32.
constexpr uint32_t TOTAL_PRESET_DATA_SIZE = 82u;

struct E4Preset final
{
    explicit E4Preset(const E4TOCChunk& chunk, BinaryReader& reader);
    explicit E4Preset(const BankPreset& preset);

    void write(BinaryWriter& writer) const;

    [[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_index); }
    [[nodiscard]] uint16_t GetNumVoices() const { return _byteswap_ushort(m_numVoices); }
    [[nodiscard]] uint16_t GetDataSize() const { return _byteswap_ushort(m_dataSize); }
    [[nodiscard]] std::string_view GetName() const { return {m_name.data(), m_name.size()}; }
    [[nodiscard]] const std::vector<E4Voice>& GetVoices() const { return m_voices; }

protected:
    void readAtLocation(ReadLocationHandle& readHandle);
    
    uint16_t m_index = 0ui16; // requires byteswap
    std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
    uint16_t m_dataSize = 0ui16; // generally 82 // requires byteswap
    uint16_t m_numVoices = 0ui16; // requires byteswap
    std::array<int8_t, 4> m_possibleRedundant1{};
    int8_t m_transpose = 0i8;
    int8_t m_volume = 0i8;
    std::array<int8_t, 24> m_possibleRedundant2{};
    std::array<int8_t, 4> m_possibleRedundant3{'R', '#', '\0', '~'};
    std::array<uint8_t, 4> m_midiControllers{255ui8, 255ui8, 255ui8, 255ui8};
    std::array<uint8_t, 24> m_possibleRedundant4{};

    /*
     * Allocated data
     */
    std::vector<E4Voice> m_voices{};
};