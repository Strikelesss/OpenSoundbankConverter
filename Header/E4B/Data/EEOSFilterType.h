#pragma once
#include <cstdint>

// TODO: More conversions
enum struct EEOSFilterType final : uint8_t
{
    TWO_POLE_LOWPASS = 1ui8,
    FOUR_POLE_LOWPASS = 0ui8,
    NO_FILTER = 127ui8,
    DREAM_WEAVA = 157ui8
};