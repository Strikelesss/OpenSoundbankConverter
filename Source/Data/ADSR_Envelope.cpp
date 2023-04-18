#include "Header/Data/ADSR_Envelope.h"
#include "Header/MathFunctions.h"

bool ADSR_Envelope::IsZeroed() const
{
    // && MathFunctions::isEqual_f(m_sustainDB, 0.f)
    return MathFunctions::isEqual_d(m_attackSec, 0.)
        && MathFunctions::isEqual_d(m_decaySec, 0.)
        && MathFunctions::isEqual_d(m_holdSec, 0.)
        && MathFunctions::isEqual_d(m_releaseSec, 0.);
}