#pragma once
#include "E4Zone.h"
#include "E4Cord.h"
#include "E4Envelope.h"
#include "EEOSFilterType.h"
#include "E4LFO.h"
#include <vector>

struct BankVoice;
struct BinaryReader;
struct E4TOCChunk;

// In the file, excluding the size of ZONE_DATA_SIZE
constexpr size_t VOICE_DATA_SIZE = 284;

constexpr size_t VOICE_DATA_READ_SIZE = 284;

constexpr uint32_t VOICE_1_ZONE_DATA_SIZE = 306u;

constexpr size_t EOS_MAX_CORDS = 24;

struct E4Voice final
{
    explicit E4Voice(const E4TOCChunk& chunk, uint16_t presetDataSize, uint16_t voiceOffset, BinaryReader& reader);
    explicit E4Voice(const BankVoice& voice);

    void write(BinaryWriter& writer) const;

    [[nodiscard]] const std::vector<E4Zone>& GetZones() const { return m_zones; }
    [[nodiscard]] const E4ZoneNoteData& GetKeyZoneRange() const { return m_keyData; }
	[[nodiscard]] const E4ZoneNoteData& GetVelocityRange() const { return m_velData; }
	[[nodiscard]] uint16_t GetVoiceDataSize() const { return _byteswap_ushort(m_totalVoiceSize); }
	[[nodiscard]] float GetChorusWidth() const;
	[[nodiscard]] float GetChorusAmount() const;
	[[nodiscard]] uint16_t GetFilterFrequency() const;
	[[nodiscard]] int8_t GetPan() const { return m_pan; }
	[[nodiscard]] int8_t GetVolume() const { return m_volume; }
	[[nodiscard]] double GetFineTune() const;
	[[nodiscard]] int8_t GetCoarseTune() const { return m_coarseTune; }
    [[nodiscard]] int8_t GetTranspose() const { return m_transpose; }
	[[nodiscard]] float GetFilterQ() const;
	[[nodiscard]] double GetKeyDelay() const;
	[[nodiscard]] EEOSFilterType GetFilterType() const;
	[[nodiscard]] const E4Envelope& GetAmpEnv() const { return m_ampEnv; }
	[[nodiscard]] const E4Envelope& GetFilterEnv() const { return m_filterEnv; }
	[[nodiscard]] const E4Envelope& GetAuxEnv() const { return m_auxEnv; }
	[[nodiscard]] const E4LFO& GetLFO1() const { return m_lfo1; }
	[[nodiscard]] const E4LFO& GetLFO2() const { return m_lfo2; }
	[[nodiscard]] const std::array<E4Cord, EOS_MAX_CORDS>& GetCords() const { return m_cords; }

    /*
    * Returns false if the cord does not exist OR if the amount is 0
    */
    [[nodiscard]] bool GetAmountFromCord(EEOSCordSource src, EEOSCordDest dst, float& outAmount) const;

    void ReplaceOrAddCord(const E4Cord& cord);
    void DisableCord(const E4Cord& cord);
    
    void PopulateCordsFromBankVoice(const BankVoice& voice);

private:
    void readAtLocation(ReadLocationHandle& readHandle);
    
	uint16_t m_totalVoiceSize = 0ui16; // requires byteswap
	int8_t m_zoneCount = 1i8;
	int8_t m_group = 0i8;
	std::array<int8_t, 8> m_amplifierData{'\0', 100i8};

	E4ZoneNoteData m_keyData;
	E4ZoneNoteData m_velData;
	E4ZoneNoteData m_rtData;

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
	std::array<int8_t, 7> m_possibleRedundant4{};
    bool m_keyLatch = false;
    std::array<int8_t, 2> m_possibleRedundant5{};
    uint8_t m_glideCurve = 0i8;
	int8_t m_volume = 0i8;
	int8_t m_pan = 0i8;
	int8_t m_possibleRedundant6 = 0i8;
	int8_t m_ampEnvDynRange = 0i8;

	EEOSFilterType m_filterType = EEOSFilterType::NO_FILTER;
	int8_t m_possibleRedundant7 = 0i8;
	uint8_t m_filterFrequency = 0ui8;
	uint8_t m_filterQ = 0ui8;

	std::array<int8_t, 48> m_possibleRedundant8{};

	E4Envelope m_ampEnv{}; // 120

	std::array<int8_t, 2> m_possibleRedundant9{}; // 122

	E4Envelope m_filterEnv{}; // 134

	std::array<int8_t, 2> m_possibleRedundant10{}; // 136

	E4Envelope m_auxEnv{}; // 148

	std::array<int8_t, 2> m_possibleRedundant11{}; // 150

	E4LFO m_lfo1{}; // 158
	E4LFO m_lfo2{}; // 166

	std::array<int8_t, 22> m_possibleRedundant12{}; // 188

    std::array<E4Cord, EOS_MAX_CORDS> m_cords = {
        E4Cord(EEOSCordSource::VEL_POLARITY_LESS, EEOSCordDest::AMP_VOLUME, 0ui8), E4Cord(EEOSCordSource::PITCH_WHEEL, EEOSCordDest::PITCH, 0ui8),
        E4Cord(EEOSCordSource::LFO1_POLARITY_CENTER, EEOSCordDest::PITCH, 0ui8), E4Cord(EEOSCordSource::MOD_WHEEL, EEOSCordDest::CORD_3_AMT, 7ui8),
        E4Cord(EEOSCordSource::VEL_POLARITY_LESS, EEOSCordDest::FILTER_FREQ, 0ui8), E4Cord(EEOSCordSource::FILTER_ENV_POLARITY_POS, EEOSCordDest::FILTER_FREQ, 0ui8),
        E4Cord(EEOSCordSource::KEY_POLARITY_CENTER, EEOSCordDest::FILTER_FREQ, 0ui8), E4Cord(EEOSCordSource::FOOTSWITCH_1, EEOSCordDest::KEY_SUSTAIN, 127ui8)}; // 284

    /*
     * Allocated data
     */
    std::vector<E4Zone> m_zones{};
};