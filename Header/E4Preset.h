#pragma once
#include "E4BFunctions.h"
#include "E4BVariables.h"

namespace PresetDefinitions
{
	constexpr double MAX_FREQUENCY_20000 = 9.90348755253612804;
	constexpr double MIN_FREQUENCY_57 = 4.04305126783455015;

	[[nodiscard]] inline std::string GetMIDINoteFromKey(const uint32_t key)
	{
		switch(key)
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

			default:{ return "null"; }
		}
	}

	[[nodiscard]] inline bool GetFilterTypeFromByte(const unsigned char b, std::string_view& outType)
	{
		const auto byteToInt(static_cast<unsigned int>(b));

		// TODO: more conversions
		switch(byteToInt)
		{
			case 127u: { outType = E4BVariables::filterTypes[0]; return true; }
			case 0u: { outType = E4BVariables::filterTypes[2]; return true; }
			case 1u: { outType = E4BVariables::filterTypes[1]; return true; }
			case 157u: { outType = E4BVariables::filterTypes[49]; return true; }
			default:{ return false;}
		}

		/*
		if (byteToInt != 127)
		{
			if (byteToInt > 0 && byteToInt < E4BVariables::FILTER_TYPE_COUNT)
			{
				outType = E4BVariables::filterTypes[byteToInt];
				return true;
			}
		}
		else
		{
			outType = E4BVariables::filterTypes[0];
			return true;
		}

		return false;
		*/
	}

	[[nodiscard]] inline int ConvertByteToFilterFrequency(const std::uint8_t b)
	{
		const double t(static_cast<double>(b) / 255.);
		return static_cast<int>(std::round(std::exp(t * (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) + MIN_FREQUENCY_57)));
	}

	[[nodiscard]] inline double ConvertByteToFineTune(const std::int8_t b)
	{
		// TODO: calculate fine tune (-100 = 192, 100 = 64)
		// values for bytes: 1.56 = 1, 3.13 = 2
		return 1.562666666666 * b;
	}

	[[nodiscard]] inline int ConvertByteToVolume(const std::uint8_t b)
	{
		return static_cast<char>(b);
	}
}

constexpr auto TOTAL_VOICE_EXTRACTION_SIZE = 306;

struct E4Voice final
{
	[[nodiscard]] std::pair<uint32_t, uint32_t> GetZoneRange() const
	{
		return std::make_pair(static_cast<uint32_t>(m_lowZone),
			static_cast<uint32_t>(m_highZone));
	}

	// todo: right source side for voice processing
	// todo: Q% (present in flys alan)
	// todo: classic response
	// todo: attack/release/etc times
	// todo: chorus

	[[nodiscard]] uint32_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] int GetFilterFrequency() const { return PresetDefinitions::ConvertByteToFilterFrequency(m_filterFrequency); }
	[[nodiscard]] int GetPan() const { return m_pan; }
	[[nodiscard]] int GetVolume() const { return PresetDefinitions::ConvertByteToVolume(m_volume); }
	[[nodiscard]] double GetFineTune() const { return PresetDefinitions::ConvertByteToFineTune(m_fineTune); }
	[[nodiscard]] bool GetFilterType(std::string_view& outType) const { return PresetDefinitions::GetFilterTypeFromByte(m_filterType, outType); }

	std::array<char, 9> m_possibleRedundantA{};
	std::array<char, 2> m_totalVoiceSize{};
	std::array<char, 10> m_possibleRedundantB{};
	unsigned char m_lowZone = 0;
	unsigned char m_lowFade = 0;
	unsigned char m_highFade = 0;
	unsigned char m_highZone = 0;
	std::array<char, 18> m_possibleRedundantC{};
	char m_fineTune = 0;
	std::array<char, 17> m_possibleRedundantD{};
	unsigned char m_volume = 0;
	char m_pan = 0;
	std::array<char, 2> m_possibleRedundantE{};
	unsigned char m_filterType = 0;
	char m_possibleRedundantF = 0;
	unsigned char m_filterFrequency = 0;
	std::array<char, 237> m_possibleRedundantG{};
	unsigned char m_originalKey = 0;

	char m_data[];
};

struct E4Preset final
{
	[[nodiscard]] uint32_t GetNumVoices() const { return m_numVoices; }

	std::array<char, E4BVariables::NAME_SIZE> m_name{};
	std::array<char, 3> m_possibleRedundantA{};
	unsigned char m_numVoices = 0;
	std::array<char, 53> m_possibleRedundantB{};
	char m_data[];
};

struct E4Emst final
{
	[[nodiscard]] uint32_t GetCurrentPreset() const { return m_currentPreset; }

	std::array<char, 4> m_name{};
	std::array<char, 27> m_possibleRedundantA{};
	unsigned char m_currentPreset = 0;
};

/*
enum struct EMIDINote
{
	G8 = 127,
	C8 = 120,
	C3 = 60,
	C0 = 24,
	C_SHARP_NEG_1 = 13,
	C_NEG_1 = 12,
	C_NEG_2 = 0
};
*/

struct E4VoiceResult final
{
	explicit E4VoiceResult(const uint32_t originalKey, const int32_t filterFreq, const int32_t pan, const int32_t volume, const double fineTune,
		const std::string_view filterType, const std::pair<uint32_t, uint32_t> zone) : m_zone(zone), m_filterType(filterType),
		m_fineTune(fineTune), m_volume(volume), m_pan(pan), m_filterFrequency(filterFreq), m_originalKey(originalKey) {}

	[[nodiscard]] const std::pair<uint32_t, uint32_t>& GetZoneRange() const { return m_zone; }
	[[nodiscard]] uint32_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] int32_t GetFilterFrequency() const { return m_filterFrequency; }
	[[nodiscard]] int32_t GetPan() const { return m_pan; }
	[[nodiscard]] int32_t GetVolume() const { return m_volume; }
	[[nodiscard]] double GetFineTune() const { return m_fineTune; }
	[[nodiscard]] const std::string_view& GetFilterType() const { return m_filterType; }

	std::pair<uint32_t, uint32_t> m_zone{0u, 0u};
	std::string_view m_filterType;
	double m_fineTune = 0.;
	int32_t m_volume = 0;
	int32_t m_pan = 0;
	int32_t m_filterFrequency = 0;
	uint32_t m_originalKey = 0u;
};

struct E4PresetResult final
{
	explicit E4PresetResult(const std::string name) : m_name(name) {}
	[[nodiscard]] const std::vector<E4VoiceResult>& GetVoices() const { return m_voices; }
	[[nodiscard]] const std::string& GetName() const { return m_name; }

	std::string m_name;
	std::vector<E4VoiceResult> m_voices{};
};

struct E4Result final
{
	E4Result() = default;
	explicit E4Result(std::vector<E4PresetResult>&& presets) : m_presets(std::move(presets)) {}
	[[nodiscard]] const std::vector<E4PresetResult>& GetPresets() const { return m_presets; }

	void Clear() { m_presets.clear(); }

	uint32_t m_currentPreset = 255u;
	std::vector<E4PresetResult> m_presets{};
};