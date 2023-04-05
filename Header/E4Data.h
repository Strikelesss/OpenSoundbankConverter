#pragma once
#include "E4BVariables.h"

struct BinaryWriter;

/*
 * Data
 */

enum EEOSCordSource : uint8_t
{
    SRC_OFF = 0ui8,
	KEY_POLARITY_POS = 8ui8,
	KEY_POLARITY_CENTER = 9ui8,
	VEL_POLARITY_POS = 10ui8,
	VEL_POLARITY_CENTER = 11ui8,
	VEL_POLARITY_LESS = 12ui8,
	PITCH_WHEEL = 16ui8,
	MOD_WHEEL = 17ui8,
	PRESSURE = 18ui8,
	PEDAL = 19ui8,
	MIDI_A = 20ui8,
	MIDI_B = 21ui8,
	FOOTSWITCH_1 = 22ui8,
	FILTER_ENV_POLARITY_POS = 80ui8,
	LFO1_POLARITY_CENTER = 96ui8
};

enum EEOSCordDest : uint8_t
{
    DST_OFF = 0ui8,
	KEY_SUSTAIN = 8ui8,
	PITCH = 48ui8,
	FILTER_FREQ = 56ui8,
	FILTER_RES = 57ui8,
	AMP_VOLUME = 64ui8,
	AMP_PAN = 65ui8,
	AMP_ENV_ATTACK = 73ui8,
	FILTER_ENV_ATTACK = 81ui8,
	CORD_3_AMT = 170ui8 // Otherwise known as 'Vibrato'
};

struct E4VoiceNoteData final
{
	E4VoiceNoteData() = default;
	explicit E4VoiceNoteData(const uint8_t low, const uint8_t high) : m_low(low), m_high(high) {}
	explicit E4VoiceNoteData(const uint8_t low, const uint8_t lowFade, const uint8_t highFade, const uint8_t high)
		: m_low(low), m_lowFade(lowFade), m_highFade(highFade), m_high(high) {}

	[[nodiscard]] uint8_t GetLow() const { return m_low; }
	[[nodiscard]] uint8_t GetHigh() const { return m_high; }
	[[nodiscard]] bool write(BinaryWriter& writer);
	void SetLow(const uint8_t low) { m_low = low; }
	void SetHigh(const uint8_t high) { m_high = high; }

private:
	uint8_t m_low = 0ui8;
	uint8_t m_lowFade = 0ui8;
	uint8_t m_highFade = 0ui8;
	uint8_t m_high = 127ui8;
};

struct E4VoiceEndData final
{
	E4VoiceEndData() = default;
	explicit E4VoiceEndData(const uint8_t sampleIndex, const uint8_t originalKey) : m_sampleIndex(sampleIndex), m_originalKey(originalKey) {}

	[[nodiscard]] const E4VoiceNoteData& GetKeyZoneRange() const { return m_keyData; }
	[[nodiscard]] const E4VoiceNoteData& GetVelocityRange() const { return m_velData; }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] uint8_t GetSampleIndex() const { return m_sampleIndex; }
	[[nodiscard]] uint8_t GetOriginalKey() const { return m_originalKey; }
	[[nodiscard]] bool write(BinaryWriter& writer);

private:
	E4VoiceNoteData m_keyData;
	E4VoiceNoteData m_velData;

	int8_t m_possibleRedundant1 = 0i8;
	uint8_t m_sampleIndex = 0ui8;
	int8_t m_possibleRedundant2 = 0i8;
	int8_t m_fineTune = 0i8;

	uint8_t m_originalKey = 0ui8;
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	int8_t m_possibleRedundant3 = 0i8;
};

constexpr auto VOICE_END_DATA_SIZE = 22ull;
constexpr auto VOICE_END_DATA_READ_SIZE = 16ull;

struct E4Envelope final
{
	E4Envelope() = default;
    explicit E4Envelope(float attackSec, float decaySec, float holdSec, float releaseSec, float delaySec, float sustainLevel);
	explicit E4Envelope(double attackSec, double decaySec, double holdSec, double releaseSec, double delaySec, float sustainLevel);

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
	/*
	 * Uses the ADSR envelope as defined in the Emulator X3 manual
	 */

	uint8_t m_attack1Sec = 0ui8;
	int8_t m_attack1Level = 0i8;
	uint8_t m_attack2Sec = 0ui8;
	int8_t m_attack2Level = 127i8;

	uint8_t m_decay1Sec = 0ui8;
	int8_t m_decay1Level = 127i8;
	uint8_t m_decay2Sec = 0ui8;
	int8_t m_decay2Level = 127i8;

	uint8_t m_release1Sec = 0ui8;
	int8_t m_release1Level = 0i8;
	uint8_t m_release2Sec = 0ui8;
	int8_t m_release2Level = 0i8;
};

struct E4LFO final
{
	E4LFO() = default;
	explicit E4LFO(double rate, uint8_t shape, double delay, bool keySync);

	[[nodiscard]] bool write(BinaryWriter& writer);
	[[nodiscard]] double GetRate() const;
	[[nodiscard]] double GetDelay() const;
	[[nodiscard]] uint8_t GetShape() const { return m_shape; }
	[[nodiscard]] bool IsKeySync() const { return !m_keySync; }
private:
	uint8_t m_rate = 13ui8;
	uint8_t m_shape = 0ui8;
	uint8_t m_delay = 0ui8;
	uint8_t m_variation = 0ui8;
	bool m_keySync = false; // 00 = on, 01 = off (make sure to flip when getting)

	std::array<int8_t, 3> m_possibleRedundant1{};
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
	uint8_t m_src = SRC_OFF;
	uint8_t m_dst = DST_OFF;
	int8_t m_amt = 0i8;
	uint8_t m_possibleRedundant1 = 0ui8;
};

struct E4Voice final
{
	E4Voice() = default;
	explicit E4Voice(float chorusWidth, float chorusAmount, uint16_t filterFreq, int8_t coarseTune, int8_t pan, int8_t volume, double fineTune, float keyDelay,
		float filterQ, std::pair<uint8_t, uint8_t> zone, std::pair<uint8_t, uint8_t> velocity, E4Envelope&& ampEnv, E4Envelope&& filterEnv, E4LFO lfo1);

	[[nodiscard]] const E4VoiceNoteData& GetKeyZoneRange() const { return m_keyData; }
	[[nodiscard]] const E4VoiceNoteData& GetVelocityRange() const { return m_velData; }
	[[nodiscard]] uint16_t GetVoiceDataSize() const { return _byteswap_ushort(m_totalVoiceSize); }
	[[nodiscard]] float GetChorusWidth() const;
	[[nodiscard]] float GetChorusAmount() const;
	[[nodiscard]] uint16_t GetFilterFrequency() const;
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] int8_t GetCoarseTune() const { return m_coarseTune; }
	[[nodiscard]] float GetFilterQ() const;
	[[nodiscard]] double GetKeyDelay() const;
	[[nodiscard]] std::string_view GetFilterType() const;
	[[nodiscard]] const E4Envelope& GetAmpEnv() const { return m_ampEnv; }
	[[nodiscard]] const E4Envelope& GetFilterEnv() const { return m_filterEnv; }
	[[nodiscard]] const E4Envelope& GetAuxEnv() const { return m_auxEnv; }
	[[nodiscard]] const E4LFO& GetLFO1() const { return m_lfo1; }
	[[nodiscard]] const E4LFO& GetLFO2() const { return m_lfo2; }
	[[nodiscard]] const std::array<E4Cord, 24>& GetCords() const { return m_cords; }
	[[nodiscard]] bool write(BinaryWriter& writer);

	void ReplaceOrAddCord(uint8_t src, uint8_t dst, float amount);
    void DisableCord(uint8_t src, uint8_t dst);

private:
	uint16_t m_totalVoiceSize = 0ui16; // requires byteswap
	int8_t m_zone = 1i8;
	int8_t m_group = 0i8;
	std::array<int8_t, 8> m_amplifierData{'\0', 100i8};

	E4VoiceNoteData m_keyData;
	E4VoiceNoteData m_velData;
	E4VoiceNoteData m_rtData;

	int8_t m_possibleRedundant1 = 0i8;
	uint8_t m_keyAssignGroup = 0ui8;
	uint16_t m_keyDelay = 0ui16; // requires byteswap
	std::array<int8_t, 3> m_possibleRedundant2{};
	uint8_t m_sampleOffset = 0ui8; // percent

	int8_t m_transpose = 0i8;
	int8_t m_coarseTune = 0i8;
	int8_t m_fineTune = 0i8;
	uint8_t m_glideRate = 0ui8;
	bool m_fixedPitch = false;
	uint8_t m_keyMode = 0ui8;
	int8_t m_possibleRedundant3 = 0i8;
	uint8_t m_chorusWidth = 128ui8;

	int8_t m_chorusAmount = 128i8;
	std::array<int8_t, 11> m_possibleRedundant4{};
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	int8_t m_possibleRedundant5 = 0i8;
	int8_t m_ampEnvDynRange = 0i8;

	uint8_t m_filterType = 127ui8;
	int8_t m_possibleRedundant6 = 0i8;
	uint8_t m_filterFrequency = 0ui8;
	uint8_t m_filterQ = 0ui8;

	std::array<int8_t, 48> m_possibleRedundant7{};

	E4Envelope m_ampEnv{}; // 120

	std::array<int8_t, 2> m_possibleRedundant8{}; // 122

	E4Envelope m_filterEnv{}; // 134

	std::array<int8_t, 2> m_possibleRedundant9{}; // 136

	E4Envelope m_auxEnv{}; // 148

	std::array<int8_t, 2> m_possibleRedundant10{}; // 150

	E4LFO m_lfo1{}; // 158
	E4LFO m_lfo2{}; // 166

	std::array<int8_t, 22> m_possibleRedundant11{}; // 188

	std::array<E4Cord, 24> m_cords{E4Cord(VEL_POLARITY_LESS, AMP_VOLUME, 0ui8), E4Cord(PITCH_WHEEL, PITCH, 1ui8), E4Cord(LFO1_POLARITY_CENTER, PITCH, 0ui8), E4Cord(MOD_WHEEL, CORD_3_AMT, 7ui8),
		E4Cord(VEL_POLARITY_LESS, FILTER_FREQ, 0ui8), E4Cord(FILTER_ENV_POLARITY_POS, FILTER_FREQ, 0ui8), E4Cord(KEY_POLARITY_CENTER, FILTER_FREQ, 0ui8), E4Cord(FOOTSWITCH_1, KEY_SUSTAIN, 127ui8)}; // 284

	int32_t m_padding = 0u; // 288
};

// In the file, excluding the 'end' size of VOICE_END_DATA_SIZE
constexpr auto VOICE_DATA_SIZE = 284ull;

constexpr auto VOICE_DATA_READ_SIZE = 284ull;

struct E4Preset final
{
	[[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_index); }
	[[nodiscard]] std::string GetName() const { return std::string(m_name.data()); }
	[[nodiscard]] uint16_t GetNumVoices() const { return _byteswap_ushort(m_numVoices); }
	[[nodiscard]] uint16_t GetDataSize() const { return _byteswap_ushort(m_dataSize); }

private:
	uint16_t m_index = 0ui16; // requires byteswap
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
	uint16_t m_dataSize = 0ui16; // generally 82 // requires byteswap
	uint16_t m_numVoices = 0ui16; // requires byteswap
	std::array<int8_t, 4> m_possibleRedundant1{};
	int8_t m_transpose = 0i8;
	int8_t m_volume = 0i8;
	std::array<int8_t, 28> m_possibleRedundant2{};
	std::array<uint8_t, 4> m_controllers{255ui8, 255ui8, 255ui8, 255ui8};

	int32_t m_padding = 0u;
};

constexpr auto TOTAL_PRESET_DATA_SIZE = 82ull;
constexpr auto PRESET_DATA_READ_SIZE = 58ull;

struct E4Sequence final
{
	[[nodiscard]] std::string GetName() const
	{
	    // Remove characters from 12 onward
	    // TODO: Get total string length from chunk
	    return std::string(m_name.data()).substr(0, 12);
	}

private:
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
};

constexpr auto SEQUENCE_DATA_READ_SIZE = 16ull;

struct E4MIDIChannel final
{
private:
	int8_t m_volume = 127i8;
	int8_t m_pan = 0i8;
	std::array<uint8_t, 3> m_possibleRedundant1{};
	uint8_t m_aux = 255ui8; // 255 = on
	std::array<uint8_t, 16> m_controllers{};
	std::array<uint8_t, 8> m_possibleRedundant2{'\0', '\0', '\0', '\0', 127ui8};
	uint16_t m_presetNum = 65535ui16; // 65535 = none
};

struct E4EMSt final
{
	[[nodiscard]] uint8_t GetCurrentPreset() const { return m_currentPreset; }
	[[nodiscard]] bool write(BinaryWriter& writer);
private:
	std::array<int8_t, 2> m_possibleRedundant1{};
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{'U', 'n', 't', 'i', 't', 'l', 'e', 'd', ' ', 'M', 'S', 'e', 't', 'u', 'p', ' '};
	std::array<int8_t, 5> m_possibleRedundant2{'\0', '\0', '\2'};
	uint8_t m_currentPreset = 0ui8;
	std::array<E4MIDIChannel, 32> m_midiChannels{};
	std::array<uint8_t, 5> m_possibleRedundant3{255ui8, 255ui8};
	int8_t m_tempo = 20i8;
	std::array<uint8_t, 312> m_possibleRedundant4{'\0', '\0', '\0', '\0', 255ui8, 255ui8, 255ui8, 255ui8};

	std::array<int8_t, 2> m_padding{};
};

constexpr auto TOTAL_EMST_DATA_SIZE = 1366ull;
constexpr auto EMST_DATA_READ_SIZE = 1366ull;