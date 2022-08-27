#pragma once
#include "E4Data.h"
#include <unordered_map>
#include <vector>

struct E4VoiceResult final
{
	explicit E4VoiceResult(const E4Voice& voice, const E4VoiceNoteData& keyZoneRange, const uint8_t originalKey, const uint8_t sampleIndex, const int8_t volume, const int8_t pan, const double fineTune) : m_keyData(keyZoneRange), m_velData(voice.GetVelocityRange()), m_filterType(voice.GetFilterType()), m_fineTune(fineTune),
		m_coarseTune(voice.GetCoarseTune()), m_filterQ(voice.GetFilterQ()), m_volume(volume), m_pan(pan), m_filterFrequency(voice.GetFilterFrequency()), m_chorusAmount(voice.GetChorusAmount()), m_chorusWidth(voice.GetChorusWidth()), m_originalKey(originalKey), m_sampleIndex(sampleIndex),
		m_keyDelay(voice.GetKeyDelay()), m_ampEnv(voice.GetAmpEnv()), m_filterEnv(voice.GetFilterEnv()), m_auxEnv(voice.GetAuxEnv()), m_lfo1(voice.GetLFO1()), m_lfo2(voice.GetLFO2()), m_cords(voice.GetCords()) {}

	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] const E4VoiceNoteData& GetKeyZoneRange() const { return m_keyData; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] const E4VoiceNoteData& GetVelocityRange() const { return m_velData; }
	[[nodiscard]] uint16_t GetFilterFrequency() const { return m_filterFrequency; }
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] float GetChorusAmount() const { return m_chorusAmount; }
	[[nodiscard]] float GetChorusWidth() const { return m_chorusWidth; }
	[[nodiscard]] float GetFilterQ() const { return m_filterQ; }
	[[nodiscard]] double GetFineTune() const { return m_fineTune; }
	[[nodiscard]] int8_t GetCoarseTune() const { return m_coarseTune; }
	[[nodiscard]] double GetKeyDelay() const { return m_keyDelay; }
	[[nodiscard]] const std::string_view& GetFilterType() const { return m_filterType; }
	[[nodiscard]] const E4Envelope& GetAmpEnv() const { return m_ampEnv; }
	[[nodiscard]] const E4Envelope& GetFilterEnv() const { return m_filterEnv; }
	[[nodiscard]] const E4Envelope& GetAuxEnv() const { return m_auxEnv; }
	[[nodiscard]] const E4LFO& GetLFO1() const { return m_lfo1; }
	[[nodiscard]] const E4LFO& GetLFO2() const { return m_lfo2; }
	[[nodiscard]] const std::array<E4Cord, E4BVariables::EOS_MAX_CORDS>& GetCords() const { return m_cords; }

	/*
	* Returns false if the cord does not exist OR if the amount is 0
	*/
	[[nodiscard]] bool GetAmountFromCord(uint8_t src, uint8_t dst, float& outAmount) const;

private:
	E4VoiceNoteData m_keyData;
	E4VoiceNoteData m_velData;
	std::string_view m_filterType;
	double m_fineTune = 0.;
	int8_t m_coarseTune = 0i8;
	float m_filterQ = 0.f;
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	uint16_t m_filterFrequency = 0ui16;
	float m_chorusAmount = 0.f;
	float m_chorusWidth = 0.f;
	uint8_t m_originalKey = 0ui8;
	uint8_t m_sampleIndex = 0ui8;
	double m_keyDelay = 0.;

	E4Envelope m_ampEnv{};
	E4Envelope m_filterEnv{};
	E4Envelope m_auxEnv{};

	E4LFO m_lfo1{};
	E4LFO m_lfo2{};

	std::array<E4Cord, E4BVariables::EOS_MAX_CORDS> m_cords{};
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
	explicit E4PresetResult(const uint16_t index, std::string&& name) : m_index(index), m_name(std::move(name)) {}

	void AddVoice(E4VoiceResult&& voice) { m_voices.emplace_back(std::move(voice)); }
	[[nodiscard]] const std::vector<E4VoiceResult>& GetVoices() const { return m_voices; }
	[[nodiscard]] const std::string& GetName() const { return m_name; }
	[[nodiscard]] uint16_t GetIndex() const { return m_index; }

private:
	uint16_t m_index = 0ui16;
	std::string m_name;
	std::vector<E4VoiceResult> m_voices{};
};

struct E4SequenceResult final
{
	explicit E4SequenceResult(std::string&& name, std::vector<char>&& data) : m_name(std::move(name)), m_midiSeqData(std::move(data)) {}
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

	void Clear() { m_presets.clear(); m_samples.clear(); m_sequences.clear(); }
	void SetCurrentPreset(const uint8_t preset) { m_currentPreset = preset; }
	E4PresetResult& AddPreset(E4PresetResult&& preset) { return m_presets.emplace_back(std::move(preset)); }
	void MapSampleIndex(const uint16_t sampleIndex, const uint8_t sampleIndex2) { m_sampleIndexMapping[sampleIndex] = sampleIndex2; }
	void AddSample(E4SampleResult&& sample) { m_samples.emplace_back(std::move(sample)); }
	void AddSequence(E4SequenceResult&& seq) { m_sequences.emplace_back(std::move(seq)); }

	[[nodiscard]] const std::vector<E4PresetResult>& GetPresets() const { return m_presets; }
	[[nodiscard]] const std::vector<E4SampleResult>& GetSamples() const { return m_samples; }
	[[nodiscard]] const std::vector<E4SequenceResult>& GetSequences() const { return m_sequences; }
	[[nodiscard]] const std::unordered_map<uint16_t, uint8_t>& GetSampleIndexMapping() const { return m_sampleIndexMapping; }
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }

private:
	uint8_t m_currentPreset = 255ui8;
	std::vector<E4PresetResult> m_presets{};
	std::vector<E4SampleResult> m_samples{};
	std::unordered_map<uint16_t, uint8_t> m_sampleIndexMapping{};
	std::vector<E4SequenceResult> m_sequences{};
};