#pragma once
#include <cstdint>
#include <string>

namespace VoiceDefinitions
{
	constexpr double MAX_FREQUENCY_20000 = 9.90348755253612804;
	constexpr double MIN_FREQUENCY_57 = 4.04305126783455015;

	[[nodiscard]] std::string GetMIDINoteFromKey(uint32_t key);
	[[nodiscard]] std::string_view GetFilterTypeFromByte(uint8_t b);
	[[nodiscard]] uint16_t ConvertByteToFilterFrequency(uint8_t b);
	[[nodiscard]] double ConvertByteToFineTune(int8_t b);
	[[nodiscard]] double ConvertByteToFilterQ(uint8_t b);

	[[nodiscard]] constexpr float GetBottomSectionPercent(const uint8_t value)
	{
		return static_cast<float>(value) * 100.f / 127.f;
	}
}