#include "Header/VoiceDefinitions.h"
#include "Header/E4BVariables.h"
#include "Header/MathFunctions.h"
#include <cmath>

int16_t VoiceDefinitions::convert_dB_to_cB(const float db)
{
	return static_cast<int16_t>(std::roundf(db * 10.f));
}

double VoiceDefinitions::centsToHertz(const int16_t cents)
{
	return 8.176 * std::pow(2., static_cast<double>(cents) / 1200.);
}

int16_t VoiceDefinitions::hertzToCents(const double hz)
{
	return static_cast<int16_t>(std::round(std::log(hz / 8.176) / std::log(2.) * 1200.));
}

std::string_view VoiceDefinitions::GetMIDINoteFromKey(const uint8_t key)
{
	if(key > 127ui8) { return "null"; }
	return E4BVariables::midiKeyNotes[key];
}

std::string_view VoiceDefinitions::GetFilterTypeFromByte(const uint8_t b)
{
	// TODO: more conversions
	switch (b)
	{
		case 127ui8: { return E4BVariables::filterTypes[0]; }
		case 0ui8: { return E4BVariables::filterTypes[2]; }
		case 1ui8: { return E4BVariables::filterTypes[1]; }
		case 157ui8: { return E4BVariables::filterTypes[49]; }
		default: { return "null"; }
	}
}

uint16_t VoiceDefinitions::ConvertByteToFilterFrequency(const std::uint8_t b)
{
	const double t(static_cast<double>(b) / 255.);
	return static_cast<uint16_t>(std::round(std::exp(t * (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) + MIN_FREQUENCY_57)));
}

uint8_t VoiceDefinitions::ConvertFilterFrequencyToByte(const uint16_t freq)
{
	return static_cast<uint8_t>(std::round((std::log(freq) - MIN_FREQUENCY_57) / (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) * 255.));
}

// [-100, 100] to [0, 64]
int8_t VoiceDefinitions::ConvertFineTuneToByte(const double fineTune)
{
	return static_cast<int8_t>((fineTune - 100.) / 1.5625 + 64.);
}

// [0, 64] to [-100, 100]
double VoiceDefinitions::ConvertByteToFineTune(const int8_t b)
{
	return MathFunctions::round_d_places((static_cast<double>(b) - 64.) * 1.5625 + 100., 2u);
}

// [0, 127] to [0.08, 18.01]
double VoiceDefinitions::GetLFORateFromByte(const uint8_t b)
{
	constexpr auto a1(1.64054); constexpr auto b1(1.01973); constexpr auto c1(-1.57702);
	return a1 * std::pow(b1, b) + c1;
}

// [0.08, 18.01] to [0, 127]
uint8_t VoiceDefinitions::GetByteFromLFORate(const double rate)
{
	constexpr auto a1(1.64054); constexpr auto b1(1.01973); constexpr auto c1(-1.57702);
	return static_cast<uint8_t>(std::round(std::log((rate - c1) / a1) / std::log(b1)));
}

// [-128, 0] to [0%, 100%]
float VoiceDefinitions::GetChorusWidthPercent(const uint8_t value)
{
	return MathFunctions::clamp_f(MathFunctions::round_f_places(std::abs((static_cast<float>(value) - 128.f) * 0.78125f), 2u), 0.f, 100.f);
}

// [0%, 100%] to [-128, 0]
uint8_t VoiceDefinitions::ConvertChorusWidthToByte(const float value)
{
	return static_cast<int8_t>(value / 0.78125f + 128.f);
}

// [0%, 100%] to [0, 127]
int8_t VoiceDefinitions::ConvertPercentToByteF(const float value)
{
	return static_cast<int8_t>(std::roundf(value * 127.f / 100.f));
}

double VoiceDefinitions::GetLFODelayFromByte(const uint8_t b)
{
	constexpr auto a1(0.149998); constexpr auto b1(1.04); constexpr auto c1(-0.150012);
	return a1 * std::pow(b1, b) + c1;
}

uint8_t VoiceDefinitions::GetByteFromLFODelay(const double delay)
{
	constexpr auto a1(0.149998); constexpr auto b1(1.04); constexpr auto c1(-0.150012);
	return static_cast<uint8_t>(std::round(std::log((delay - c1) / a1) / std::log(b1)));
}

double VoiceDefinitions::GetTimeFromCurveAttack(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.084 * static_cast<double>(b - 59ui8));
}

uint8_t VoiceDefinitions::GetByteFromSecAttack(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.084 + 59.));
}

double VoiceDefinitions::GetTimeFromCurveDecay1(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.015 * static_cast<double>(b - 59ui8));
}

uint8_t VoiceDefinitions::GetByteFromSecDecay1(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.015 + 59.));
}

double VoiceDefinitions::GetTimeFromCurveDecay2(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.1 * static_cast<double>(b - 59ui8));
}

uint8_t VoiceDefinitions::GetByteFromSecDecay2(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.1 + 59.));
}

double VoiceDefinitions::GetTimeFromCurveRelease(const uint8_t b)
{
	return 1.3 * std::pow(2., 0.1 * static_cast<double>(b - 59ui8));
}

uint8_t VoiceDefinitions::GetByteFromSecRelease(const double sec)
{
	return static_cast<uint8_t>(std::abs(std::log2(sec / 1.3) / 0.1 + 59.));
}