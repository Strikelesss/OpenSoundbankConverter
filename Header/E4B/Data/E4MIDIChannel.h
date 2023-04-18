#pragma once
#include <array>

struct ReadLocationHandle;
struct BinaryWriter;

struct E4MIDIChannel final
{
    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
private:
    int8_t m_volume = 127i8;
    int8_t m_pan = 0i8;
    std::array<uint8_t, 3> m_possibleRedundant1{};
    uint8_t m_aux = 255ui8; // 255 = on
    std::array<uint8_t, 16> m_controllers{};
    std::array<uint8_t, 8> m_possibleRedundant2{'\0', '\0', '\0', '\0', 127ui8};
    uint16_t m_presetNum = 65535ui16; // 65535 = none
};