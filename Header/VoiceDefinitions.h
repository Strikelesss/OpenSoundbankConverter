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
	[[nodiscard]] uint8_t ConvertFilterFrequencyToByte(uint16_t freq);
	[[nodiscard]] int8_t ConvertFineTuneToByte(double fineTune);
	[[nodiscard]] double ConvertByteToFineTune(int8_t b);
	[[nodiscard]] float ConvertByteToFilterQ(uint8_t b);
	[[nodiscard]] double GetTimeFromCurveAttack(uint8_t b);
	[[nodiscard]] double GetTimeFromCurveRelease(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromSecAttack(double sec);
	[[nodiscard]] uint8_t GetByteFromSecRelease(double sec);

	[[nodiscard]] float GetBottomSectionPercent(uint8_t value);
	[[nodiscard]] int8_t ConvertPercentToByteF(float value, bool inverted = false);
	[[nodiscard]] int8_t ConvertPercentToByteD(double value);
	[[nodiscard]] float ConvertByteToPercentF(int8_t b);
}