#pragma once
#include <array>
#include <cstdint>

struct ReadLocationHandle;
struct BinaryReader;
struct BinaryWriter;

constexpr uint64_t ZONE_DATA_SIZE = 22ull;
constexpr uint64_t ZONE_DATA_READ_SIZE = 16ull;
constexpr uint64_t ZONE_NOTE_DATA_SIZE = 4ull;

struct E4ZoneNoteData final
{
    E4ZoneNoteData() = default;
    explicit E4ZoneNoteData(const uint8_t low, const uint8_t high) : m_low(low), m_high(high) {}
    explicit E4ZoneNoteData(const uint8_t low, const uint8_t lowFade, const uint8_t highFade, const uint8_t high)
        : m_low(low), m_lowFade(lowFade), m_highFade(highFade), m_high(high) {}

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);

    [[nodiscard]] uint8_t GetLow() const { return m_low; }
    [[nodiscard]] uint8_t GetHigh() const { return m_high; }
    void SetLow(const uint8_t low) { m_low = low; }
    void SetHigh(const uint8_t high) { m_high = high; }

private:
    uint8_t m_low = 0ui8;
    uint8_t m_lowFade = 0ui8;
    uint8_t m_highFade = 0ui8;
    uint8_t m_high = 127ui8;
};

struct E4Zone final
{
    E4Zone() = default;
    explicit E4Zone(const uint16_t sampleIndex, const uint8_t originalKey)
        : m_sampleIndex(_byteswap_ushort(sampleIndex)), m_originalKey(originalKey) {}

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] const E4ZoneNoteData& GetKeyZoneRange() const { return m_keyData; }
    [[nodiscard]] const E4ZoneNoteData& GetVelocityRange() const { return m_velData; }
    [[nodiscard]] double GetFineTune() const;
    [[nodiscard]] int8_t GetVolume() const { return m_volume; }
    [[nodiscard]] int8_t GetPan() const { return m_pan; }
    [[nodiscard]] uint16_t GetSampleIndex() const { return _byteswap_ushort(m_sampleIndex); }
    [[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
    
protected:
    E4ZoneNoteData m_keyData;
    E4ZoneNoteData m_velData;
    
    uint16_t m_sampleIndex = 0ui16; // requires byteswap
    int8_t m_possibleRedundant1 = 0i8;
    int8_t m_fineTune = 0i8; // Could be uint16, but it the voice fineTune follows int8.

    uint8_t m_originalKey = 0ui8;
    int8_t m_volume = 0i8;
    int8_t m_pan = 0i8;
    std::array<int8_t, 7> m_possibleRedundant2{};
};