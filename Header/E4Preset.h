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

private:
	uint8_t m_lowZone = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant1{};
	uint8_t m_highZone = 0ui8;

	std::array<int8_t, 5> m_possibleRedundant2{};
	uint8_t m_sampleIndex = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant3{};

	uint8_t m_originalKey = 0ui8;
	std::array<int8_t, 3> m_padding{};
};

constexpr auto VOICE_END_DATA_SIZE = 22ull;
constexpr auto VOICE_END_DATA_READ_SIZE = 13ull;

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
	// 12 = 0.108
	// 32 = 0.411
	// 59 = 1.976
	// 84 = 8.045
	// 97 = 16.872
	// 111 = 40.119
	// 120 = 79.558
	// 127 = 163.60

	// todo: right source side for voice processing
	// todo: attack/release/etc times

	[[nodiscard]] uint16_t GetVoiceDataSize() const { return _byteswap_ushort(m_totalVoiceSize); }
	[[nodiscard]] uint8_t GetChorusWidth() const { return m_chorusWidth; }
	[[nodiscard]] uint8_t GetChorusAmount() const { return m_chorusAmount; }
	[[nodiscard]] uint16_t GetFilterFrequency() const;
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return static_cast<int8_t>(m_volume); }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] double GetFilterQ() const;
	[[nodiscard]] std::string_view GetFilterType() const;
	[[nodiscard]] float GetAttack1Level() const;
	[[nodiscard]] float GetAttack2Level() const;
	[[nodiscard]] float GetDecay1Level() const;
	[[nodiscard]] float GetDecay2Level() const;
	[[nodiscard]] float GetRelease1Level() const;
	[[nodiscard]] float GetRelease2Level() const;

private:
	uint16_t m_totalVoiceSize = 0i16;
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

	std::array<int8_t, 48> m_possibleRedundant9{};

	uint8_t m_attack1Sec = 0ui8;
	uint8_t m_attack1Level = 0ui8;
	uint8_t m_attack2Sec = 0ui8;
	uint8_t m_attack2Level = 0ui8;

	uint8_t m_decay1Sec = 0ui8;
	uint8_t m_decay1Level = 0ui8;
	uint8_t m_decay2Sec = 0ui8;
	uint8_t m_decay2Level = 0ui8;

	uint8_t m_release1Sec = 0ui8;
	uint8_t m_release1Level = 0ui8;
	uint8_t m_release2Sec = 0ui8;
	uint8_t m_release2Level = 0ui8;
};

// In the file, excluding the 'end' size of VOICE_END_DATA_SIZE
constexpr auto VOICE_DATA_SIZE = 284ull;

constexpr auto VOICE_DATA_READ_SIZE = 120ull;

struct E4Preset final
{
	[[nodiscard]] std::string GetPresetName() const { return std::string(m_name); }
	[[nodiscard]] uint16_t GetNumVoices() const { return _byteswap_ushort(m_numVoices); }
	[[nodiscard]] uint16_t GetPresetDataSize() const { return _byteswap_ushort(m_presetDataSize); }

private:
	std::array<char, E4BVariables::NAME_SIZE> m_name{};
	uint16_t m_presetDataSize = 0ui16;
	uint16_t m_numVoices = 0ui16;

	std::array<int16_t, 2> m_padding{};
};

constexpr auto PRESET_DATA_READ_SIZE = 20ull;

struct E4Sequence final
{
	[[nodiscard]] std::string GetName() const { return std::string(m_name); }

private:
	std::array<char, E4BVariables::NAME_SIZE> m_name{};
};

struct E4Emst final
{
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }

private:
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
		const std::pair<uint8_t, uint8_t> velocity, const float attack1Level, const float attack2Level, const float decay1Level, const float decay2Level, const float release1Level, 
		const float release2Level) : m_zone(zone), m_velocity(velocity), m_filterType(filterType), m_fineTune(fineTune), m_filterQ(filterQ), m_volume(volume), m_pan(pan),
		m_filterFrequency(filterFreq), m_chorusAmount(chorusAmount), m_chorusWidth(chorusWidth), m_originalKey(originalKey), m_sampleIndex(sampleIndex), m_attack1Level(attack1Level),
		m_attack2Level(attack2Level), m_decay1Level(decay1Level), m_decay2Level(decay2Level), m_release1Level(release1Level), m_release2Level(release2Level) {}

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
	[[nodiscard]] float GetAttack1Level() const { return m_attack1Level; }
	[[nodiscard]] float GetAttack2Level() const { return m_attack2Level; }
	[[nodiscard]] float GetDecay1Level() const { return m_decay1Level; }
	[[nodiscard]] float GetDecay2Level() const { return m_decay2Level; }
	[[nodiscard]] float GetRelease1Level() const { return m_release1Level; }
	[[nodiscard]] float GetRelease2Level() const { return m_release2Level; }

private:
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

	float m_attack1Sec = 0.f;
	float m_attack1Level = 0.f;
	float m_attack2Sec = 0.f;
	float m_attack2Level = 0.f;
	float m_decay1Sec = 0.f;
	float m_decay1Level = 0.f;
	float m_decay2Sec = 0.f;
	float m_decay2Level = 0.f;
	float m_release1Sec = 0.f;
	float m_release1Level = 0.f;
	float m_release2Sec = 0.f;
	float m_release2Level = 0.f;
};

struct E4SampleResult final
{
	explicit E4SampleResult(std::string&& sampleName, std::vector<int16_t>&& data, const uint32_t sampleRate, const uint32_t channels, const bool isLooping, const bool isReleasing, const uint32_t loopStart, const uint32_t loopEnd)
		: m_sampleName(std::move(sampleName)), m_sampleData(std::move(data)), m_sampleRate(sampleRate), m_loopStart(loopStart), m_loopEnd(loopEnd), m_channels(channels), m_isLooping(isLooping), m_isReleasing(isReleasing) {}

	[[nodiscard]] const std::string& GetName() const { return m_sampleName; }
	[[nodiscard]] uint32_t GetSampleRate() const { return m_sampleRate; }
	[[nodiscard]] uint32_t GetLoopStart() const { return m_loopStart; }
	[[nodiscard]] uint32_t GetLoopEnd() const { return m_loopEnd; }
	[[nodiscard]] uint32_t GetNumChannels() const { return m_channels; }
	[[nodiscard]] bool IsLooping() const { return m_isLooping; }
	[[nodiscard]] bool IsReleasing() const { return m_isReleasing; }
	[[nodiscard]] const std::vector<int16_t>& GetData() const { return m_sampleData; }

private:
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
	explicit E4PresetResult(std::string&& name) : m_name(std::move(name)) {}
	[[nodiscard]] const std::vector<E4VoiceResult>& GetVoices() const { return m_voices; }
	[[nodiscard]] const std::string& GetName() const { return m_name; }

	void AddVoice(E4VoiceResult&& voice) { m_voices.emplace_back(std::move(voice)); }

private:
	std::string m_name;
	std::vector<E4VoiceResult> m_voices{};
};

struct E4SequenceResult final
{
	explicit E4SequenceResult(std::string name, std::vector<char>&& data) : m_name(std::move(name)), m_midiSeqData(std::move(data)) {}
	[[nodiscard]] const std::vector<char>& GetMIDISeqData() const { return m_midiSeqData; }
	[[nodiscard]] const std::string& GetName() const { return m_name; }

private:
	std::string m_name;
	std::vector<char> m_midiSeqData{};
};

struct E4Result final
{
	E4Result() = default;
	explicit E4Result(std::vector<E4PresetResult>&& presets) : m_presets(std::move(presets)) {}
	[[nodiscard]] const std::vector<E4PresetResult>& GetPresets() const { return m_presets; }
	[[nodiscard]] const std::vector<E4SampleResult>& GetSamples() const { return m_samples; }
	[[nodiscard]] const std::vector<E4SequenceResult>& GetSequences() const { return m_sequences; }
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }

	void Clear() { m_presets.clear(); m_samples.clear(); m_sequences.clear(); }
	void SetCurrentPreset(const uint8_t preset) { m_currentPreset = preset; }
	E4PresetResult& AddPreset(E4PresetResult&& preset) { return m_presets.emplace_back(std::move(preset)); }
	void AddSample(E4SampleResult&& sample) { m_samples.emplace_back(std::move(sample)); }
	void AddSequence(E4SequenceResult&& seq) { m_sequences.emplace_back(std::move(seq)); }

private:
	uint8_t m_currentPreset = 255ui8;
	std::vector<E4PresetResult> m_presets{};
	std::vector<E4SampleResult> m_samples{};
	std::vector<E4SequenceResult> m_sequences{};
};