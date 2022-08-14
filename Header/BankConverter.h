#pragma once
#include <filesystem>

struct E4Result;

struct ConverterOptions final
{
	ConverterOptions() = default;
	explicit ConverterOptions(const bool flipPan, const bool isChickenTranslatorFile, const bool ignoreFileNameSetting = false)
		: m_flipPan(flipPan), m_isChickenTranslatorFile(isChickenTranslatorFile), m_ignoreFileNameSetting(ignoreFileNameSetting) {}

	bool m_flipPan = false;
	bool m_isChickenTranslatorFile = false;
	bool m_ignoreFileNameSetting = false;
	std::filesystem::path m_ignoreFileNameSettingSaveFolder;
};

namespace SF2Converter
{
	/*
	* Converter specific
	*/

	// kUnused1 = LFO1 ~ -> Amp Pan
	// kUnused2 = Positive or negative attenuation
	// kUnused3 = LFO1 shape
	// kUnused4 = Filter Env Attack
	// kUnused5 = LFO1 ~ Key Sync

	constexpr auto SF2_MAX_NAME_LEN = 20u;
	constexpr auto BASE_CENT_VALUE = 6900i16;
	constexpr auto SF2_FILTER_MIN_FREQ = 1500i16;
	constexpr auto SF2_FILTER_MAX_FREQ = 13500i16;

	[[nodiscard]] int16_t FilterFrequencyToCents(uint16_t freq);
	[[nodiscard]] int16_t secToTimecent(double sec);
}

struct BankConverter final
{
	[[nodiscard]] bool ConvertE4BToSF2(const E4Result& e4b, const std::string_view& bankName, const ConverterOptions& options = {}) const;
	[[nodiscard]] bool ConvertSF2ToE4B(const std::filesystem::path& bank, const std::string_view& bankName, const ConverterOptions& options = {}) const;
private:
	[[nodiscard]] std::string ConvertNameToEmuName(const std::string_view& name) const;
	[[nodiscard]] std::string ConvertNameToSFName(const std::string_view& name) const;
};