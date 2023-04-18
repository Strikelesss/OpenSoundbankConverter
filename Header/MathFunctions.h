#pragma once
#include <cstdint>

namespace MathFunctions
{
    [[nodiscard]] bool isEqual_f(float a, float b);
    [[nodiscard]] bool isEqual_d(double a, double b);
	[[nodiscard]] float clamp_f(float value, float min, float max);
	[[nodiscard]] double round_d_places(double value, uint32_t places);
	[[nodiscard]] float round_f_places(float value, uint32_t places);
    [[nodiscard]] uint32_t byteswapUINT32(uint32_t value);
}