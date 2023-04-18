#pragma once
#include "Header/E4B/Helpers/E4BVariables.h"
#include "E4MIDIChannel.h"
#include <array>

struct ReadLocationHandle;
struct BinaryWriter;

constexpr auto TOTAL_EMST_DATA_SIZE = 1366ull;
constexpr auto EMST_DATA_READ_SIZE = 1366ull;

struct E4EMSt final
{
    E4EMSt() = default;
    explicit E4EMSt(const uint8_t defaultPreset) : m_currentPreset(defaultPreset) {}
    
    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }
    
private:
    std::array<int8_t, 2> m_possibleRedundant1{};
    std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{'U', 'n', 't', 'i', 't', 'l', 'e', 'd', ' ', 'M', 'S', 'e', 't', 'u', 'p', ' '};
    std::array<int8_t, 5> m_possibleRedundant2{'\0', '\0', '\2'};
    uint8_t m_currentPreset = 0ui8;
    std::array<E4MIDIChannel, 32> m_midiChannels{};
    std::array<uint8_t, 5> m_possibleRedundant3{255ui8, 255ui8};
    int8_t m_tempo = 20i8;
    std::array<uint8_t, 312> m_possibleRedundant4{'\0', '\0', '\0', '\0', 255ui8, 255ui8, 255ui8, 255ui8};
};