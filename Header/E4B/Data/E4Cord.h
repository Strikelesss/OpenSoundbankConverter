#pragma once
#include <cstdint>

struct ReadLocationHandle;
struct BinaryReader;
struct BinaryWriter;

enum struct EEOSCordSource final : uint8_t
{
    SRC_OFF = 0ui8,
    KEY_POLARITY_POS = 8ui8,
    KEY_POLARITY_CENTER = 9ui8,
    VEL_POLARITY_POS = 10ui8,
    VEL_POLARITY_CENTER = 11ui8,
    VEL_POLARITY_LESS = 12ui8,
    PITCH_WHEEL = 16ui8,
    MOD_WHEEL = 17ui8,
    PRESSURE = 18ui8,
    PEDAL = 19ui8,
    MIDI_A = 20ui8,
    MIDI_B = 21ui8,
    FOOTSWITCH_1 = 22ui8,
    FILTER_ENV_POLARITY_POS = 80ui8,
    LFO1_POLARITY_CENTER = 96ui8
};

enum struct EEOSCordDest final : uint8_t
{
    DST_OFF = 0ui8,
    KEY_SUSTAIN = 8ui8,
    PITCH = 48ui8,
    FILTER_FREQ = 56ui8,
    FILTER_RES = 57ui8,
    AMP_VOLUME = 64ui8,
    AMP_PAN = 65ui8,
    AMP_ENV_ATTACK = 73ui8,
    FILTER_ENV_ATTACK = 81ui8,
    CORD_3_AMT = 170ui8 // Otherwise known as 'Vibrato'
};

struct E4Cord final
{
    E4Cord() = default;
    explicit E4Cord(const EEOSCordSource src, const EEOSCordDest dst, const int8_t amt) : m_src(src), m_dst(dst), m_amt(amt) {}

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] EEOSCordSource GetSource() const { return m_src; }
    [[nodiscard]] EEOSCordDest GetDest() const { return m_dst; }
    [[nodiscard]] int8_t GetAmount() const { return m_amt; }
    void SetAmount(const int8_t amount) { m_amt = amount; }
    void SetSrc(const EEOSCordSource src) { m_src = src; }
    void SetDst(const EEOSCordDest dst) { m_dst = dst; }
private:
    EEOSCordSource m_src = EEOSCordSource::SRC_OFF;
    EEOSCordDest m_dst = EEOSCordDest::DST_OFF;
    int8_t m_amt = 0i8;
    uint8_t m_possibleRedundant1 = 0ui8;
};