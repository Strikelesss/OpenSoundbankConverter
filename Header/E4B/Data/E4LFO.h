#pragma once
#include <array>

struct ReadLocationHandle;
struct BinaryWriter;
constexpr uint64_t E4_LFO_DATA_SIZE = 8ull;

struct E4LFO final
{
    E4LFO() = default;
    explicit E4LFO(double rate, uint8_t shape, double delay, bool keySync);

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] double GetRate() const;
    [[nodiscard]] double GetDelay() const;
    [[nodiscard]] uint8_t GetShape() const { return m_shape; }
    [[nodiscard]] bool IsKeySync() const { return !m_keySync; }
private:
    uint8_t m_rate = 13ui8;
    uint8_t m_shape = 0ui8;
    uint8_t m_delay = 0ui8;
    uint8_t m_variation = 0ui8;
    bool m_keySync = false; // 00 = on, 01 = off (make sure to flip when getting)

    std::array<int8_t, 3> m_possibleRedundant1{};
};