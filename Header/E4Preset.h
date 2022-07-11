#pragma once
#include "E4BFunctions.h"
#include "E4BVariables.h"

namespace VoiceDefinitions
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

	[[nodiscard]] inline std::string_view GetFilterTypeFromByte(const uint8_t b)
	{
		const auto byteToInt(static_cast<uint32_t>(b));

		// TODO: more conversions
		switch(byteToInt)
		{
			case 127u: { return E4BVariables::filterTypes[0]; }
			case 0u: { return E4BVariables::filterTypes[2]; }
			case 1u: { return E4BVariables::filterTypes[1]; }
			case 157u: { return E4BVariables::filterTypes[49]; }
			default:{ return "null"; }
		}
	}

	[[nodiscard]] inline uint16_t ConvertByteToFilterFrequency(const std::uint8_t b)
	{
		const double t(static_cast<double>(b) / 255.);
		return static_cast<uint16_t>(std::round(std::exp(t * (MAX_FREQUENCY_20000 - MIN_FREQUENCY_57) + MIN_FREQUENCY_57)));
	}

	[[nodiscard]] inline double ConvertByteToFineTune(const std::int8_t b)
	{
		// TODO: calculate fine tune (-100 = 192, 100 = 64)
		// values for bytes: 1.56 = 1, 3.13 = 2
		return 1.562666666666 * b;
	}

	[[nodiscard]] inline double ConvertByteToFilterQ(const std::uint8_t b)
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
}

struct E4VoiceEndData final
{
	[[nodiscard]] std::pair<uint8_t, uint8_t> GetZoneRange() const
	{
		return std::make_pair(static_cast<uint8_t>(m_lowZone),
			static_cast<uint8_t>(m_highZone));
	}

	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }

	uint8_t m_lowZone = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant1{};
	uint8_t m_highZone = 0ui8;
	std::array<int8_t, 5> m_possibleRedundant2{};
	uint8_t m_sampleIndex = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant3{};
	uint8_t m_originalKey = 0ui8;
	std::array<int8_t, 9> m_possibleRedundant4{};
};

struct E4Voice final
{
	[[nodiscard]] std::pair<uint8_t, uint8_t> GetZoneRange() const
	{
		return std::make_pair(static_cast<uint8_t>(m_lowZone),
			static_cast<uint8_t>(m_highZone));
	}

	[[nodiscard]] std::pair<uint8_t, uint8_t> GetVelocityRange() const
	{
		return std::make_pair(m_minVelocity, m_maxVelocity);
	}

	// release values:
	// 0 = 0
	// 59 = 1.976
	// 84 = 8.045
	// 97 = 16.872
	// 111 = 40.119
	// 120 = 79.558
	// 127 = 163.60

	// todo: right source side for voice processing
	// todo: attack/release/etc times

	[[nodiscard]] uint8_t GetChorusWidth() const { return m_chorusWidth; }
	[[nodiscard]] uint8_t GetChorusAmount() const { return m_chorusAmount; }
	[[nodiscard]] uint16_t GetFilterFrequency() const { return VoiceDefinitions::ConvertByteToFilterFrequency(m_filterFrequency); }
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return static_cast<int8_t>(m_volume); }
	[[nodiscard]] double GetFineTune() const { return VoiceDefinitions::ConvertByteToFineTune(m_fineTune); }
	[[nodiscard]] double GetFilterQ() const { return VoiceDefinitions::ConvertByteToFilterQ(m_filterQ); }
	[[nodiscard]] std::string_view GetFilterType() const { return VoiceDefinitions::GetFilterTypeFromByte(m_filterType); }

	std::array<int8_t, 9> m_possibleRedundant1{};
	std::array<int8_t, 2> m_totalVoiceSize{};
	std::array<int8_t, 10> m_possibleRedundant2{};
	uint8_t m_lowZone = 0ui8;
	uint8_t m_lowFade = 0ui8;
	uint8_t m_highFade = 0ui8;
	uint8_t m_highZone = 0ui8;
	uint8_t m_minVelocity = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant3{};
	uint8_t m_maxVelocity = 0ui8;
	std::array<int8_t, 14> m_possibleRedundant4{};
	int8_t m_fineTune = 0i8;
	std::array<int8_t, 4> m_possibleRedundant5{};
	int8_t m_chorusWidth = 0i8;
	int8_t m_chorusAmount = 0i8;
	std::array<int8_t, 11> m_possibleRedundant6{};
	uint8_t m_volume = 0ui8;
	int8_t m_pan = 0i8;
	std::array<int8_t, 2> m_possibleRedundant7{};
	uint8_t m_filterType = 0ui8;
	int8_t m_possibleRedundant8 = 0i8;
	uint8_t m_filterFrequency = 0ui8;
	uint8_t m_filterQ = 0ui8;
	std::array<int8_t, 224> m_possibleRedundant9{};
};

struct E4Preset final
{
	[[nodiscard]] uint32_t GetNumVoices() const { return m_numVoices; }

	std::array<char, E4BVariables::NAME_SIZE> m_name{};
	std::array<int8_t, 3> m_possibleRedundantA{};
	uint8_t m_numVoices = 0ui8;
	std::array<int8_t, 53> m_possibleRedundantB{};
};

struct E4Emst final
{
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }

	std::array<char, 4> m_name{};
	std::array<int8_t, 27> m_possibleRedundantA{};
	uint8_t m_currentPreset = 0ui8;
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
	explicit E4VoiceResult(const uint8_t sampleIndex, const uint8_t originalKey, const uint8_t chorusWidth, const uint8_t chorusAmount, const uint16_t filterFreq, 
		const int8_t pan, const int8_t volume, const double fineTune, const double filterQ, const std::string_view filterType, const std::pair<uint8_t, uint8_t> zone, 
		const std::pair<uint8_t, uint8_t> velocity) : m_zone(zone), m_velocity(velocity), m_filterType(filterType), m_fineTune(fineTune), m_filterQ(filterQ), m_volume(volume),
		m_pan(pan), m_filterFrequency(filterFreq), m_chorusAmount(chorusAmount), m_chorusWidth(chorusWidth), m_originalKey(originalKey), m_sampleIndex(sampleIndex) {}

	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] const std::pair<uint8_t, uint8_t>& GetZoneRange() const { return m_zone; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] const std::pair<uint8_t, uint8_t>& GetVelocityRange() const { return m_velocity; }
	[[nodiscard]] uint16_t GetFilterFrequency() const { return m_filterFrequency; }
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] uint8_t GetChorusAmount() const { return m_chorusAmount; }
	[[nodiscard]] uint8_t GetChorusWidth() const { return m_chorusWidth; }
	[[nodiscard]] double GetFilterQ() const { return m_filterQ; }
	[[nodiscard]] double GetFineTune() const { return m_fineTune; }
	[[nodiscard]] const std::string_view& GetFilterType() const { return m_filterType; }

	std::pair<uint8_t, uint8_t> m_zone{0ui8, 0ui8};
	std::pair<uint8_t, uint8_t> m_velocity{0ui8, 0ui8};
	std::string_view m_filterType;
	double m_fineTune = 0.;
	double m_filterQ = 0.;
	int8_t m_volume = 0;
	int8_t m_pan = 0;
	uint16_t m_filterFrequency = 0;
	uint8_t m_chorusAmount = 0ui8;
	uint8_t m_chorusWidth = 0ui8;
	uint8_t m_originalKey = 0ui8;
	uint8_t m_sampleIndex = 0ui8;
};

struct E4SampleResult final
{
	explicit E4SampleResult(std::string&& sampleName, std::vector<int16_t>&& data, const uint32_t sampleRate, const uint32_t channels, const bool isLooping, const bool isReleasing, const uint32_t loopStart, const uint32_t loopEnd)
		: m_sampleName(std::move(sampleName)), m_sampleData(std::move(data)), m_sampleRate(sampleRate), m_loopStart(loopStart), m_loopEnd(loopEnd), m_channels(channels), m_isLooping(isLooping), m_isReleasing(isReleasing) {}

	std::string m_sampleName;
	std::vector<int16_t> m_sampleData{};
	uint32_t m_sampleRate = 0u;
	uint32_t m_loopStart = 0u;
	uint32_t m_loopEnd = 0u;
	uint32_t m_channels = 0u;
	bool m_isLooping = false;
	bool m_isReleasing = false;
};

struct E4PresetResult final
{
	explicit E4PresetResult(std::string name) : m_name(std::move(name)) {}
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
	[[nodiscard]] const std::vector<E4SampleResult>& GetSamples() const { return m_samples; }

	void Clear() { m_presets.clear(); m_samples.clear(); }

	uint8_t m_currentPreset = 255ui8;
	std::vector<E4PresetResult> m_presets{};
	std::vector<E4SampleResult> m_samples{};
};