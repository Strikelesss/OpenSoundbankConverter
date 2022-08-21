#pragma once
#include <cstdint>
#include <string>

namespace VoiceDefinitions
{
	constexpr double MAX_FREQUENCY_20000 = 9.90348755253612804;
	constexpr double MIN_FREQUENCY_57 = 4.04305126783455015;

	constexpr float convert_cB_to_dB(const float cb) { return cb / 10.f; }
	int16_t convert_dB_to_cB(float db);

	double centsToHertz(int16_t cents);
	int16_t hertzToCents(double hz);

	[[nodiscard]] std::string GetMIDINoteFromKey(uint32_t key);
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

	[[nodiscard]] float GetBottomSectionPercent(uint8_t value);
	[[nodiscard]] float GetChorusWidthPercent(uint8_t value);
	[[nodiscard]] constexpr uint8_t ConvertChorusWidthToByte(const float value) { return static_cast<uint8_t>(value * 128.f / 100.f + 128.f); }
	[[nodiscard]] int8_t ConvertPercentToByteF(float value);
	[[nodiscard]] int8_t ConvertPercentToByteD(double value);
	[[nodiscard]] float ConvertByteToPercentF(int8_t b);
}