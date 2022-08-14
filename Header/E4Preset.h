#pragma once
#include "E4BVariables.h"
#include <unordered_map>
#include <vector>

struct BinaryWriter;

/*
 * Data
 */

enum EEOSCordSource : uint8_t
{
	VELOCITY_POLARITY_POS = 10ui8,
	VELOCITY_POLARITY_LESS = 12ui8,
	PITCH_WHEEL = 16ui8,
	MIDI_A = 20ui8,
	MIDI_B = 21ui8,
	FILTER_ENV_POLARITY_POS = 80ui8,
	LFO1_POLARITY_CENTER = 96ui8
};

enum EEOSCordDest : uint8_t
{
	PITCH = 48ui8,
	FILTER_FREQ = 56ui8,
	FILTER_RES = 57ui8,
	AMP_VOLUME = 64ui8,
	AMP_PAN = 65ui8,
	FILTER_ENV_ATTACK = 81ui8
};

struct E4VoiceEndData final
{
	E4VoiceEndData() = default;
	explicit E4VoiceEndData(const uint8_t sampleIndex, const uint8_t originalKey) : m_sampleIndex(sampleIndex), m_originalKey(originalKey) {}

	[[nodiscard]] std::pair<uint8_t, uint8_t> GetZoneRange() const { return std::make_pair(m_lowZone, m_highZone); }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] bool write(BinaryWriter& writer);

private:
	int8_t m_lowZone = 0i8;
	std::array<int8_t, 2> m_possibleRedundant1{};
	int8_t m_highZone = 127i8;

	std::array<int8_t, 5> m_possibleRedundant2{'\0', '\0', '\0', static_cast<char>(127)};
	uint8_t m_sampleIndex = 0ui8;
	int8_t m_possibleRedundant3 = 0i8;
	int8_t m_fineTune = 0i8;

	uint8_t m_originalKey = 0ui8;
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	int8_t m_possibleRedundant4 = 0i8;
};

constexpr auto VOICE_END_DATA_SIZE = 22ull;
constexpr auto VOICE_END_DATA_READ_SIZE = 16ull;

struct E4Envelope final
{
	E4Envelope() = default;
	explicit E4Envelope(double attack, double release);

	[[nodiscard]] double GetAttack1Sec() const;
	[[nodiscard]] float GetAttack1Level() const;
	[[nodiscard]] double GetAttack2Sec() const;
	[[nodiscard]] float GetAttack2Level() const;
	[[nodiscard]] double GetDecay1Sec() const;
	[[nodiscard]] float GetDecay1Level() const;
	[[nodiscard]] double GetDecay2Sec() const;
	[[nodiscard]] float GetDecay2Level() const;
	[[nodiscard]] double GetRelease1Sec() const;
	[[nodiscard]] float GetRelease1Level() const;
	[[nodiscard]] double GetRelease2Sec() const;
	[[nodiscard]] float GetRelease2Level() const;
	[[nodiscard]] bool write(BinaryWriter& writer);

private:
	uint8_t m_attack1Sec = 0ui8;
	int8_t m_attack1Level = 127ui8;
	uint8_t m_attack2Sec = 0ui8;
	int8_t m_attack2Level = 127ui8;

	uint8_t m_decay1Sec = 0ui8;
	int8_t m_decay1Level = 127ui8;
	uint8_t m_decay2Sec = 0ui8;
	int8_t m_decay2Level = 127ui8;

	uint8_t m_release1Sec = 0ui8;
	int8_t m_release1Level = 0ui8;
	uint8_t m_release2Sec = 0ui8;
	int8_t m_release2Level = 0ui8;
};

struct E4LFO final
{
	E4LFO() = default;

	// TODO: convert delay to uint8_t
	explicit E4LFO(const uint8_t rate, const uint8_t shape, const double delay, const bool keySync) : m_rate(rate), m_shape(shape), m_delay(static_cast<uint8_t>(delay)), m_keySync(!keySync) {}

	[[nodiscard]] bool write(BinaryWriter& writer);
	[[nodiscard]] uint8_t GetRate() const { return m_rate; }
	[[nodiscard]] uint8_t GetDelay() const { return m_delay; }
	[[nodiscard]] uint8_t GetShape() const { return m_shape; }
	[[nodiscard]] bool IsKeySync() const { return !m_keySync; }
private:
	uint8_t m_rate = 13ui8;
	uint8_t m_shape = 0ui8;
	uint8_t m_delay = 0ui8;
	uint8_t m_variation = 0ui8;
	bool m_keySync = true; // 00 = on, 01 = off (make sure to flip when getting)

	std::array<int8_t, 3> m_possibleRedundant2{};
};

struct E4Cord final
{
	E4Cord() = default;
	explicit E4Cord(const uint8_t src, const uint8_t dst, const int8_t amt) : m_src(src), m_dst(dst), m_amt(amt) {}

	[[nodiscard]] uint8_t GetSource() const { return m_src; }
	[[nodiscard]] int8_t GetAmount() const { return m_amt; }
	[[nodiscard]] uint8_t GetDest() const { return m_dst; }
	void SetAmount(const int8_t amount) { m_amt = amount; }
	void SetSrc(const uint8_t src) { m_src = src; }
	void SetDst(const uint8_t dst) { m_dst = dst; }
private:
	uint8_t m_src = 0ui8;
	uint8_t m_dst = 0ui8;
	int8_t m_amt = 0ui8;
	uint8_t m_possibleRedundant1 = 0ui8;
};

struct E4Voice final
{
	E4Voice() = default;

	explicit E4Voice(float chorusWidth, float chorusAmount, uint16_t filterFreq, int8_t coarseTune, int8_t pan, int8_t volume, double fineTune, double keyDelay,
		float filterQ, std::pair<uint8_t, uint8_t> zone, std::pair<uint8_t, uint8_t> velocity, E4Envelope&& ampEnv, E4Envelope&& filterEnv, E4LFO&& lfo1);

	[[nodiscard]] std::pair<uint8_t, uint8_t> GetZoneRange() const
	{
		return std::make_pair(m_lowZone, m_highZone);
	}

	[[nodiscard]] std::pair<uint8_t, uint8_t> GetVelocityRange() const
	{
		return std::make_pair(m_minVelocity, m_maxVelocity);
	}

	[[nodiscard]] uint16_t GetVoiceDataSize() const { return _byteswap_ushort(m_totalVoiceSize); }
	[[nodiscard]] float GetChorusWidth() const;
	[[nodiscard]] float GetChorusAmount() const;
	[[nodiscard]] uint16_t GetFilterFrequency() const;
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] int8_t GetCoarseTune() const { return m_coarseTune; }
	[[nodiscard]] float GetFilterQ() const;
	[[nodiscard]] double GetKeyDelay() const { return static_cast<double>(_byteswap_ushort(m_keyDelay)) / 1000.; }
	[[nodiscard]] std::string_view GetFilterType() const;
	[[nodiscard]] const E4Envelope& GetAmpEnv() const { return m_ampEnv; }
	[[nodiscard]] const E4Envelope& GetFilterEnv() const { return m_filterEnv; }
	[[nodiscard]] const E4Envelope& GetAuxEnv() const { return m_auxEnv; }
	[[nodiscard]] const E4LFO& GetLFO1() const { return m_lfo1; }
	[[nodiscard]] const E4LFO& GetLFO2() const { return m_lfo2; }
	[[nodiscard]] const std::array<E4Cord, 24>& GetCords() const { return m_cords; }
	[[nodiscard]] bool write(BinaryWriter& writer);

	void ReplaceOrAddCord(uint8_t src, uint8_t dst, float amount);

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

private:
	uint16_t m_totalVoiceSize = 0i16; // requires byteswap
	int8_t m_possibleRedundant1 = 1i8;
	int8_t m_group = 0i8;
	std::array<int8_t, 8> m_possibleRedundant2{'\0', 'd'};
	uint8_t m_lowZone = 0ui8;
	uint8_t m_lowFade = 0ui8;
	uint8_t m_highFade = 0ui8;
	uint8_t m_highZone = 0ui8;

	uint8_t m_minVelocity = 0ui8;
	std::array<int8_t, 2> m_possibleRedundant3{};
	uint8_t m_maxVelocity = 0ui8;

	std::array<int8_t, 6> m_possibleRedundant4{'\0', '\0', '\0', static_cast<char>(127)};
	uint16_t m_keyDelay = 0ui16; // requires byteswap
	std::array<int8_t, 3> m_possibleRedundant5{};
	uint8_t m_sampleOffset = 0ui8; // percent

	int8_t m_transpose = 0i8;
	int8_t m_coarseTune = 0i8;
	int8_t m_fineTune = 0i8;
	int8_t m_possibleRedundant6 = 0i8;
	bool m_fixedPitch = false;
	std::array<int8_t, 2> m_possibleRedundant7{};
	int8_t m_chorusWidth = 0i8;

	int8_t m_chorusAmount = 128i8;
	std::array<int8_t, 11> m_possibleRedundant8{};
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	std::array<int8_t, 2> m_possibleRedundant9{};

	uint8_t m_filterType = 127ui8;
	int8_t m_possibleRedundant10 = 0i8;
	uint8_t m_filterFrequency = 0ui8;
	uint8_t m_filterQ = 0ui8;

	std::array<int8_t, 48> m_possibleRedundant11{};

	E4Envelope m_ampEnv{}; // 120

	std::array<int8_t, 2> m_possibleRedundant12{}; // 122

	E4Envelope m_filterEnv{}; // 134

	std::array<int8_t, 2> m_possibleRedundant13{}; // 136

	E4Envelope m_auxEnv{}; // 148

	std::array<int8_t, 2> m_possibleRedundant14{}; // 150

	E4LFO m_lfo1{}; // 158
	E4LFO m_lfo2{}; // 166

	std::array<int8_t, 22> m_possibleRedundant15{}; // 188

	std::array<E4Cord, 24> m_cords{E4Cord(12ui8, 64ui8, 0ui8), E4Cord(16ui8, 48ui8, 1ui8), E4Cord(96ui8, 48ui8, 0ui8), E4Cord(17ui8, 170ui8, 1ui8),
		E4Cord(12ui8, 56ui8, 0ui8), E4Cord(80ui8, 56ui8, 0ui8), E4Cord(8ui8, 56ui8, 0ui8), E4Cord(22ui8, 8ui8, 127ui8)}; // 284

	std::array<int8_t, 4> m_padding{}; // 288
};

// In the file, excluding the 'end' size of VOICE_END_DATA_SIZE
constexpr auto VOICE_DATA_SIZE = 284ull;

constexpr auto VOICE_DATA_READ_SIZE = 284ull;

struct E4Preset final
{
	[[nodiscard]] std::string GetPresetName() const { return std::string(m_name); }
	[[nodiscard]] uint16_t GetNumVoices() const { return _byteswap_ushort(m_numVoices); }
	[[nodiscard]] uint16_t GetPresetDataSize() const { return _byteswap_ushort(m_presetDataSize); }

private:
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
	uint16_t m_presetDataSize = 0ui16;
	uint16_t m_numVoices = 0ui16;

	std::array<int16_t, 2> m_padding{};
};

constexpr auto PRESET_DATA_READ_SIZE = 20ull;
constexpr auto TOTAL_PRESET_DATA_SIZE = 82ull;

struct E4Sequence final
{
	[[nodiscard]] std::string GetName() const { return std::string(m_name); }

private:
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
};

constexpr auto SEQUENCE_DATA_READ_SIZE = 16ull;

// Startup?
struct E4EMSt final
{
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }
	[[nodiscard]] bool write(BinaryWriter& writer);
private:
	uint32_t m_dataSize = 0u;
	std::array<int8_t, 2> m_possibleRedundant1{};
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{'U', 'n', 't', 'i', 't', 'l', 'e', 'd', ' ', 'M', 'S', 'e', 't', 'u', 'p', ' '};
	std::array<int8_t, 5> m_possibleRedundant2{'\0', '\0', '\2'};
	uint8_t m_currentPreset = 0ui8;
	std::array<uint8_t, 28> m_possibleRedundant3{static_cast<uint8_t>(127), '\0', '\0', '\0', '\0', static_cast<uint8_t>(255),
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', static_cast<uint8_t>(127)};
};

constexpr auto TOTAL_EMST_DATA_SIZE = 56ull;
constexpr auto EMST_DATA_READ_SIZE = 28ull;

/*
 * Results
 */

struct E4VoiceResult final
{
	explicit E4VoiceResult(const E4Voice& voice, std::pair<uint8_t, uint8_t>&& zoneRange, const uint8_t originalKey, const uint8_t sampleIndex, const int8_t volume, const int8_t pan, const double fineTune, const std::array<E4Cord, 24>& cords) : m_zone(std::move(zoneRange)), m_velocity(voice.GetVelocityRange()), m_filterType(voice.GetFilterType()), m_fineTune(fineTune),
		m_coarseTune(voice.GetCoarseTune()), m_filterQ(voice.GetFilterQ()), m_volume(volume), m_pan(pan), m_filterFrequency(voice.GetFilterFrequency()), m_chorusAmount(voice.GetChorusAmount()), m_chorusWidth(voice.GetChorusWidth()), m_originalKey(originalKey), m_sampleIndex(sampleIndex),
		m_keyDelay(voice.GetKeyDelay()), m_ampEnv(voice.GetAmpEnv()), m_filterEnv(voice.GetFilterEnv()), m_auxEnv(voice.GetAuxEnv()), m_lfo1(voice.GetLFO1()), m_lfo2(voice.GetLFO2()), m_cords(cords) {}

	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] const std::pair<uint8_t, uint8_t>& GetZoneRange() const { return m_zone; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] const std::pair<uint8_t, uint8_t>& GetVelocityRange() const { return m_velocity; }
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
	[[nodiscard]] const std::array<E4Cord, 24>& GetCords() const { return m_cords; }

	/*
	 * Returns false if the cord does not exist OR if the amount is 0
	 */
	[[nodiscard]] bool GetAmountFromCord(uint8_t src, uint8_t dst, float& outAmount) const;

private:
	std::pair<uint8_t, uint8_t> m_zone{0ui8, 0ui8};
	std::pair<uint8_t, uint8_t> m_velocity{0ui8, 0ui8};
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

	std::array<E4Cord, 24> m_cords{};
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

	void AddVoice(E4VoiceResult&& voice) { m_voices.emplace_back(std::move(voice)); }
	[[nodiscard]] const std::vector<E4VoiceResult>& GetVoices() const { return m_voices; }
	[[nodiscard]] const std::string& GetName() const { return m_name; }

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