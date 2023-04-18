#pragma once

namespace ADSR_EnvelopeStatics
{
    constexpr float MIN_ADSR = 0.f;
    constexpr float MAX_ATTACK_TIME = 40.f;
    constexpr float MAX_DECAY_TIME = 80.f;
    constexpr float MAX_HOLD_TIME = 20.f;
    constexpr float MAX_SUSTAIN_DB = 100.f;
    constexpr float MAX_RELEASE_TIME = 80.f;
}

struct ADSR_Envelope final
{
    ADSR_Envelope() = default;
    explicit ADSR_Envelope(const double attackSec, const double decaySec, const double holdSec, const float sustainDB, const double releaseSec, const double delaySec)
        : m_attackSec(attackSec), m_decaySec(decaySec), m_delaySec(delaySec), m_holdSec(holdSec), m_releaseSec(releaseSec), m_sustainDB(sustainDB) {}

    [[nodiscard]] bool IsZeroed() const;
    
    double m_attackSec = 0.;
    double m_decaySec = 0.;
    double m_delaySec = 0.;
    double m_holdSec = 0.;
    double m_releaseSec = 0.;
    float m_sustainDB = 0.f;
};