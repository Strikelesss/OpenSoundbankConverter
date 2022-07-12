#include "Header/VoiceDefinitions.h"
#include "Header/E4BVariables.h"
#include <cmath>

std::string VoiceDefinitions::GetMIDINoteFromKey(const uint32_t key)
{
	switch (key)
	{
		case 127: { return "G8"; }
		case 126: { return "F#8"; }
		case 125: { return "F8"; }
		case 124: { return "E8"; }
		case 123: { return "D#8"; }
		case 122: { return "D8"; }
		case 121: { return "C#8"; }
		case 120: { return "C8"; }

		case 119: { return "B7"; }
		case 118: { return "A#7"; }
		case 117: { return "A7"; }
		case 116: { return "G#7"; }
		case 115: { return "G7"; }
		case 114: { return "F#7"; }
		case 113: { return "F7"; }
		case 112: { return "E7"; }
		case 111: { return "D#7"; }
		case 110: { return "D7"; }
		case 109: { return "C#7"; }
		case 108: { return "C7"; }

		case 107: { return "B6"; }
		case 106: { return "A#6"; }
		case 105: { return "A6"; }
		case 104: { return "G#6"; }
		case 103: { return "G6"; }
		case 102: { return "F#6"; }
		case 101: { return "F6"; }
		case 100: { return "E6"; }
		case 99: { return "D#6"; }
		case 98: { return "D6"; }
		case 97: { return "C#6"; }
		case 96: { return "C6"; }

		case 95: { return "B5"; }
		case 94: { return "A#5"; }
		case 93: { return "A5"; }
		case 92: { return "G#5"; }
		case 91: { return "G5"; }
		case 90: { return "F#5"; }
		case 89: { return "F5"; }
		case 88: { return "E5"; }
		case 87: { return "D#5"; }
		case 86: { return "D5"; }
		case 85: { return "C#5"; }
		case 84: { return "C5"; }

		case 83: { return "B4"; }
		case 82: { return "A#4"; }
		case 81: { return "A4"; }
		case 80: { return "G#4"; }
		case 79: { return "G4"; }
		case 78: { return "F#4"; }
		case 77: { return "F4"; }
		case 76: { return "E4"; }
		case 75: { return "D#4"; }
		case 74: { return "D4"; }
		case 73: { return "C#4"; }
		case 72: { return "C4"; }

		case 71: { return "B3"; }
		case 70: { return "A#3"; }
		case 69: { return "A3"; }
		case 68: { return "G#3"; }
		case 67: { return "G3"; }
		case 66: { return "F#3"; }
		case 65: { return "F3"; }
		case 64: { return "E3"; }
		case 63: { return "D#3"; }
		case 62: { return "D3"; }
		case 61: { return "C#3"; }
		case 60: { return "C3"; }

		case 59: { return "B2"; }
		case 58: { return "A#2"; }
		case 57: { return "A2"; }
		case 56: { return "G#2"; }
		case 55: { return "G2"; }
		case 54: { return "F#2"; }
		case 53: { return "F2"; }
		case 52: { return "E2"; }
		case 51: { return "D#2"; }
		case 50: { return "D2"; }
		case 49: { return "C#2"; }
		case 48: { return "C2"; }

		case 47: { return "B1"; }
		case 46: { return "A#1"; }
		case 45: { return "A1"; }
		case 44: { return "G#1"; }
		case 43: { return "G1"; }
		case 42: { return "F#1"; }
		case 41: { return "F1"; }
		case 40: { return "E1"; }
		case 39: { return "D#1"; }
		case 38: { return "D1"; }
		case 37: { return "C#1"; }
		case 36: { return "C1"; }

		case 35: { return "B0"; }
		case 34: { return "A#0"; }
		case 33: { return "A0"; }
		case 32: { return "G#0"; }
		case 31: { return "G0"; }
		case 30: { return "F#0"; }
		case 29: { return "F0"; }
		case 28: { return "E0"; }
		case 27: { return "D#0"; }
		case 26: { return "D0"; }
		case 25: { return "C#0"; }
		case 24: { return "C0"; }

		case 23: { return "B-1"; }
		case 22: { return "A#-1"; }
		case 21: { return "A-1"; }
		case 20: { return "G#-1"; }
		case 19: { return "G-1"; }
		case 18: { return "F#-1"; }
		case 17: { return "F-1"; }
		case 16: { return "E-1"; }
		case 15: { return "D#-1"; }
		case 14: { return "D-1"; }
		case 13: { return "C#-1"; }
		case 12: { return "C-1"; }

		case 11: { return "B-2"; }
		case 10: { return "A#-2"; }
		case 9: { return "A-2"; }
		case 8: { return "G#-2"; }
		case 7: { return "G-2"; }
		case 6: { return "F#-2"; }
		case 5: { return "F-2"; }
		case 4: { return "E-2"; }
		case 3: { return "D#-2"; }
		case 2: { return "D-2"; }
		case 1: { return "C#-2"; }
		case 0: { return "C-2"; }

		default: { return "null"; }
	}
}

std::string_view VoiceDefinitions::GetFilterTypeFromByte(const uint8_t b)
{
	const auto byteToInt(static_cast<uint32_t>(b));

		// TODO: more conversions
	switch (byteToInt)
	{
		case 127u: { return E4BVariables::filterTypes[0]; }
		case 0u: { return E4BVariables::filterTypes[2]; }
		case 1u: { return E4BVariables::filterTypes[1]; }
		case 157u: { return E4BVariables::filterTypes[49]; }
		default: { return "null"; }
	}
}

uint16_t VoiceDefinitions::ConvertByteToFilterFrequency(const std::uint8_t b)
{
	const double t(static_cast<double>(b) / 255.);
	return static_cast<uint16_t>(std::round(std::exp(t * (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) + MIN_FREQUENCY_57)));
}

double VoiceDefinitions::ConvertByteToFineTune(const std::int8_t b)
{
	// TODO: calculate fine tune (-100 = 192, 100 = 64)
	// values for bytes: 1.56 = 1, 3.13 = 2
	return 1.562666666666 * b;
}

double VoiceDefinitions::ConvertByteToFilterQ(const std::uint8_t b)
{
	// 127 = 100
		// 126 = 99.2
		// 125 = 98.4
		// 62 = 48.8
		// 29 = 22.8
		// 19 = 15
		// 5 = 4.7
		// 1 = 0.8
		// 0 = 0

		// TODO: calculate filter Q
	return 0.785947461 * b + 0.17062;
}