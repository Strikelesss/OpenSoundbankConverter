#pragma once
#include <filesystem>

struct E4Result;

namespace SF2Converter
{
	constexpr auto SF2_MAX_NAME_LEN = 20u;
	constexpr auto BASE_CENT_VALUE = 6900i16;
	constexpr auto SF2_FILTER_MIN_FREQ = 1500i16;
	constexpr auto SF2_FILTER_MAX_FREQ = 13500i16;

	[[nodiscard]] int16_t FilterFrequencyToCents(uint16_t freq);
	[[nodiscard]] int16_t secToTimecent(double sec);
}

struct BankConverter final
{
	[[nodiscard]] bool ConvertE4BToSF2(const E4Result& e4b, const std::string_view& bankName) const;
	[[nodiscard]] bool ConvertSF2ToE4B(const std::filesystem::path& bank, const std::string_view& bankName) const;
private:
	[[nodiscard]] std::string ConvertNameToEmuName(const std::string_view& name) const;
	[[nodiscard]] std::string ConvertNameToSFName(const std::string_view& name) const;
};