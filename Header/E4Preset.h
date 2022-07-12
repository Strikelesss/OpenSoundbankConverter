#pragma once
#include "E4BVariables.h"
#include <string_view>
#include <vector>

/*
 * Data
 */

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
	[[nodiscard]] uint16_t GetFilterFrequency() const;
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return static_cast<int8_t>(m_volume); }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] double GetFilterQ() const;
	[[nodiscard]] std::string_view GetFilterType() const;

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
 * Results
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