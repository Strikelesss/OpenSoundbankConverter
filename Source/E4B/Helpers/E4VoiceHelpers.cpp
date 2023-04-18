#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/E4B/Helpers/E4BVariables.h"
#include "Header/MathFunctions.h"
#include <cmath>

std::string_view E4VoiceHelpers::GetMIDINoteFromKey(const uint32_t key)
{
	if(key > 127ui8) { return "null"; }
	return E4BVariables::midiKeyNotes.at(key);
}

EEOSFilterType E4VoiceHelpers::GetFilterTypeFromByte(const uint8_t b)
{
	return static_cast<EEOSFilterType>(b);
}

uint16_t E4VoiceHelpers::ConvertByteToFilterFrequency(const std::uint8_t b)
{
	const double t(static_cast<double>(b) / MAX_FREQUENCY_BYTE);
	return static_cast<uint16_t>(std::round(std::exp(t * (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) + MIN_FREQUENCY_57)));
}

uint8_t E4VoiceHelpers::ConvertFilterFrequencyToByte(const uint16_t freq)
{
	return static_cast<uint8_t>(std::round((std::log(freq) - MIN_FREQUENCY_57) / (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) * MAX_FREQUENCY_BYTE));
}

// [-100, 100] to [-64, 64]
int8_t E4VoiceHelpers::ConvertFineTuneToByte(const double fineTune)
{
	return static_cast<int8_t>(std::round((fineTune - 100.) / MIN_FINE_TUNE + MAX_FINE_TUNE_BYTE));
}

// [-64, 64] to [-100, 100]
double E4VoiceHelpers::ConvertByteToFineTune(const int8_t b)
{
	return MathFunctions::round_d_places((static_cast<double>(b) - MAX_FINE_TUNE_BYTE) * MIN_FINE_TUNE + 100., 2u);
}

// [0, 127] to [0.08, 18.01]
double E4VoiceHelpers::GetLFORateFromByte(const uint8_t b)
{
	constexpr auto a1(1.64054); constexpr auto b1(1.01973); constexpr auto c1(-1.57702);
	return a1 * std::pow(b1, b) + c1;
}

// [0.08, 18.01] to [0, 127]
uint8_t E4VoiceHelpers::GetByteFromLFORate(const double rate)
{
	constexpr auto a1(1.64054); constexpr auto b1(1.01973); constexpr auto c1(-1.57702);
	return static_cast<uint8_t>(std::round(std::log((rate - c1) / a1) / std::log(b1)));
}

// [-128, 0] to [0%, 100%]
float E4VoiceHelpers::GetChorusWidthPercent(const uint8_t value)
{
	return MathFunctions::clamp_f(MathFunctions::round_f_places(std::abs((static_cast<float>(value) - 128.f) * MIN_CHORUS_WIDTH), 2u), 0.f, 100.f);
}

// [0%, 100%] to [-128, 0]
uint8_t E4VoiceHelpers::ConvertChorusWidthToByte(const float value)
{
	return static_cast<int8_t>(value / MIN_CHORUS_WIDTH + 128.f);
}

// [0%, 100%] to [0, 127]
int8_t E4VoiceHelpers::ConvertPercentToByteF(const float value)
{
	return static_cast<int8_t>(std::roundf(value * 127.f / 100.f));
}

double E4VoiceHelpers::GetLFODelayFromByte(const uint8_t b)
{
	constexpr auto a1(0.149998); constexpr auto b1(1.04); constexpr auto c1(-0.150012);
	return a1 * std::pow(b1, b) + c1;
}

uint8_t E4VoiceHelpers::GetByteFromLFODelay(const double delay)
{
	constexpr auto a1(0.149998); constexpr auto b1(1.04); constexpr auto c1(-0.150012);
	return static_cast<uint8_t>(std::round(std::log((delay - c1) / a1) / std::log(b1)));
}

double E4VoiceHelpers::GetTimeFromCurveAttack(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.084 * static_cast<double>(b - 59ui8));
}

uint8_t E4VoiceHelpers::GetByteFromSecAttack(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.084 + 59.));
}

double E4VoiceHelpers::GetTimeFromCurveDecay1(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.015 * static_cast<double>(b - 59ui8));
}

uint8_t E4VoiceHelpers::GetByteFromSecDecay1(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.015 + 59.));
}

double E4VoiceHelpers::GetTimeFromCurveDecay2(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.1 * static_cast<double>(b - 59ui8));
}

uint8_t E4VoiceHelpers::GetByteFromSecDecay2(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.1 + 59.));
}

double E4VoiceHelpers::GetTimeFromCurveRelease(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.1 * static_cast<double>(b - 59ui8));
}

uint8_t E4VoiceHelpers::GetByteFromSecRelease(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.1 + 59.));
}