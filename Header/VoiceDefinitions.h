#pragma once
#include <cstdint>
#include <string_view>

namespace VoiceDefinitions
{
	constexpr auto MAX_FREQUENCY_20000 = 9.90348755253612804;
	constexpr auto MIN_FREQUENCY_57 = 4.04305126783455015;
	constexpr auto MAX_FREQUENCY_BYTE = 255.;
	constexpr auto MAX_FINE_TUNE_BYTE = 64.;
	constexpr auto MIN_CHORUS_WIDTH = 0.78125f;
	constexpr auto MIN_FINE_TUNE = 1.5625;

	constexpr float convert_cB_to_dB(const float cb) { return cb / 10.f; }
	int16_t convert_dB_to_cB(float db);

	double centsToHertz(int16_t cents);
	int16_t hertzToCents(double hz);

	template<typename T>
	constexpr int16_t valueToRelPercent(const T val) { return static_cast<int16_t>(val * T(10)); }

	constexpr float relPercentToValue(const float val) { return val / 10.f; }

	[[nodiscard]] std::string_view GetMIDINoteFromKey(uint32_t key);
	[[nodiscard]] std::string_view GetFilterTypeFromByte(uint8_t b);
	[[nodiscard]] uint16_t ConvertByteToFilterFrequency(uint8_t b);
	[[nodiscard]] uint8_t ConvertFilterFrequencyToByte(uint16_t freq);
	[[nodiscard]] int8_t ConvertFineTuneToByte(double fineTune);
	[[nodiscard]] double ConvertByteToFineTune(int8_t b);

	[[nodiscard]] double GetLFORateFromByte(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromLFORate(double rate);

	[[nodiscard]] double GetLFODelayFromByte(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromLFODelay(double delay);

	[[nodiscard]] double GetTimeFromCurveAttack(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromSecAttack(double sec);

	[[nodiscard]] double GetTimeFromCurveDecay1(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromSecDecay1(double sec);

	[[nodiscard]] double GetTimeFromCurveDecay2(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromSecDecay2(double sec);

	[[nodiscard]] double GetTimeFromCurveRelease(uint8_t b);
	[[nodiscard]] uint8_t GetByteFromSecRelease(double sec);

	[[nodiscard]] float GetChorusWidthPercent(uint8_t value);
	[[nodiscard]] uint8_t ConvertChorusWidthToByte(float value);
	[[nodiscard]] int8_t ConvertPercentToByteF(float value);

	template<typename T>
	[[nodiscard]] constexpr float ConvertByteToPercentF(const T b)
	{
		// Make sure we're only converting bytes
		static_assert(sizeof(T) == 1);

		return static_cast<float>(b) / 127.f * 100.f;
	}
}