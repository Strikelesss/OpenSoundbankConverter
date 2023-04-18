#pragma once
#include "Header/Data/ADSR_Envelope.h"

struct E4BReadOptions final
{
    E4BReadOptions() = default;
    
    /*
     * Converts any fine tune over 50 to negative range
     * This is an Emulator IV issue with converting Emax II banks over to E4B format.
     */
    bool m_useFineTuneCorrection = false;
};

struct BankReadOptions final
{
	BankReadOptions() = default;

    // General options
    
	bool m_flipPan = false;
	bool m_useConverterSpecificData = true;
    ADSR_Envelope m_filterDefaults{};

    // E4B options
    
    E4BReadOptions m_e4bOptions{};
};