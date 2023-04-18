#pragma once
#include <cstdint>

struct ReadLocationHandle;
struct BinaryWriter;

enum struct EE4EnvelopeType final
{
    AMP, FILTER, AUX
};

constexpr uint64_t E4_ENV_DATA_SIZE = 12ull;

struct E4Envelope final
{
    E4Envelope() = default;
    explicit E4Envelope(float attackSec, float decaySec, float holdSec, float releaseSec, float delaySec, float sustainLevel);
    explicit E4Envelope(double attackSec, double decaySec, double holdSec, double releaseSec, double delaySec, float sustainLevel);

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    [[nodiscard]] double GetAttack1Sec() const;
    [[nodiscard]] float GetAttack1Level() const;
    [[nodiscard]] double GetAttack2Sec() const;
    [[nodiscard]] float GetAttack2Level() const;
    [[nodiscard]] double GetDecay1Sec() const;
    [[nodiscard]] float GetDecay1Level() const;
    [[nodiscard]] double GetDecay2Sec() const;
    [[nodiscard]] float GetDecay2Level() const;
    [[nodiscard]] double GetRelease1Sec() const;
    [[nodiscard]] float GetRelease1Level() const;
    [[nodiscard]] double GetRelease2Sec() const;
    [[nodiscard]] float GetRelease2Level() const;

private:
    /*
     * Uses the ADSR envelope as defined in the Emulator X3 manual
     */

    uint8_t m_attack1Sec = 0ui8;
    int8_t m_attack1Level = 0i8;
    uint8_t m_attack2Sec = 0ui8;
    int8_t m_attack2Level = 127i8;

    uint8_t m_decay1Sec = 0ui8;
    int8_t m_decay1Level = 127i8;
    uint8_t m_decay2Sec = 0ui8;
    int8_t m_decay2Level = 127i8;

    uint8_t m_release1Sec = 0ui8;
    int8_t m_release1Level = 0i8;
    uint8_t m_release2Sec = 0ui8;
    int8_t m_release2Level = 0i8;
};