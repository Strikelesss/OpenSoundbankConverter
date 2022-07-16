#pragma once
#include <string>

struct E4Result;

namespace SF2Converter
{
	constexpr auto BASE_CENT_VALUE = 6900i16;
	constexpr auto SF2_FILTER_MIN_FREQ = 1500i16;
	constexpr auto SF2_FILTER_MAX_FREQ = 13500i16;

	[[nodiscard]] int16_t FilterFrequencyToCents(uint16_t freq);
	[[nodiscard]] int16_t secToTimecent(double sec);
}

struct BankConverter final
{
	[[nodiscard]] bool ConvertE4BToSF2(const E4Result& e4b, const std::string& bankName) const;
};