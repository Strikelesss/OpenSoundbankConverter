#include "Header/Data/Soundbank.h"

bool BankVoice::GetAmountFromRTControl(const ERealtimeControlSrc src, const ERealtimeControlDst dst, float& outAmount) const
{
    for(const auto& control : m_realtimeControls)
    {
        if(control.m_src == src && control.m_dst == dst)
        {
            outAmount = control.m_amount;
            return true;
        }
    }

    return false;
}

void BankVoice::ReplaceOrAddRTControl(const ERealtimeControlSrc src, const ERealtimeControlDst dst, const float amount)
{
    // Replace if existing
    for(auto& control : m_realtimeControls)
    {
        if(control.m_src == src && control.m_dst == dst)
        {
            control.m_amount = amount;
            return;
        }
    }

    // Replace null cord
    for(auto& control : m_realtimeControls)
    {
        if(control.m_src == ERealtimeControlSrc::SRC_OFF && control.m_dst == ERealtimeControlDst::DST_OFF)
        {
            control.m_amount = amount;
            control.m_src = src;
            control.m_dst = dst;
            break;
        }
    }
}

void BankVoice::DisableRTControl(const ERealtimeControlSrc src, const ERealtimeControlDst dst)
{
    for(auto& control : m_realtimeControls)
    {
        if(control.m_src == src && control.m_dst == dst)
        {
            control.m_src = ERealtimeControlSrc::SRC_OFF;
            break;
        }
    }
}

void Soundbank::Clear()
{
    m_bankName.clear();
    m_presets.clear();
    m_samples.clear();
    m_sequences.clear();
    m_defaultPreset = 255ui8;
}
