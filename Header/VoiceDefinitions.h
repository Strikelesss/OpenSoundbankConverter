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

	[[nodiscard]] constexpr float GetBottomSectionPercent(const uint8_t value)
	{
		return static_cast<float>(value) * 100.f / 127.f;
	}

	[[nodiscard]] constexpr int8_t ConvertPercentToByteF(const float value, const bool inverted = false)
	{
		if(inverted) { return static_cast<int8_t>(value * 127.f / 100.f - 128.f); }
		return static_cast<int8_t>(value * 127.f / 100.f);
	}

	[[nodiscard]] constexpr int8_t ConvertPercentToByteD(const double value)
	{
		return static_cast<int8_t>(value * 127. / 100.);
	}

	[[nodiscard]] constexpr float ConvertByteToPercentF(const int8_t b)
	{
		return static_cast<float>(b) / 127.f * 100.f;
	}
}