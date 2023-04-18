#pragma once
#include <array>

#include "ADSR_Envelope.h"
#include <string>
#include <vector>

struct BankSample final
{
    explicit BankSample(const uint16_t index, std::string&& name, std::vector<int16_t>&& data, const uint32_t sampleRate,
        const uint32_t numChannels, const bool isLooping, const bool isReleasing, const uint32_t loopStart, const uint32_t loopEnd)
        : m_sampleName(std::move(name)), m_sampleData(std::move(data)), m_sampleRate(sampleRate), m_loopStart(loopStart),
        m_loopEnd(loopEnd), m_channels(numChannels), m_index(index), m_isLooping(isLooping), m_isLoopReleasing(isReleasing) {}

    std::string m_sampleName;
    std::vector<int16_t> m_sampleData{};
    uint32_t m_sampleRate = 0u;
    uint32_t m_loopStart = 0u;
    uint32_t m_loopEnd = 0u;
    uint32_t m_channels = 1u;
    uint16_t m_index = 0ui16;
    bool m_isLooping = false;
    bool m_isLoopReleasing = false;
};

struct BankNoteRange final
{
    explicit BankNoteRange(const uint8_t low, const uint8_t high) : m_low(low), m_high(high) {}
    
    uint8_t m_low = 0ui8;
    uint8_t m_high = 127ui8;
};

struct BankLFO final
{
    explicit BankLFO(const double rate, const uint8_t shape, const double delay, const bool keySync)
        : m_rate(rate), m_shape(shape), m_delay(delay), m_keySync(keySync) {}
    
    double m_rate = 13ui8;
    uint8_t m_shape = 0ui8;
    double m_delay = 0ui8;
    bool m_keySync = false; // 00 = on, 01 = off (make sure to flip when getting)
};

enum struct ERealtimeControlSrc final
{
    SRC_OFF,
    KEY_POLARITY_POS, KEY_POLARITY_CENTER,
    VEL_POLARITY_POS, VEL_POLARITY_CENTER, VEL_POLARITY_LESS,
    PITCH_WHEEL, MOD_WHEEL, PRESSURE, PEDAL, MIDI_A, MIDI_B, FOOTSWITCH_1,
    FILTER_ENV_POLARITY_POS,
    LFO1_POLARITY_CENTER
};

enum struct ERealtimeControlDst final
{
    DST_OFF,
    KEY_SUSTAIN,
    VIBRATO, PITCH,
    FILTER_FREQ, FILTER_RES,
    AMP_VOLUME, AMP_PAN, AMP_ENV_ATTACK,
    FILTER_ENV_ATTACK
};

struct BankRealtimeControl final
{
    BankRealtimeControl() = default;
    explicit BankRealtimeControl(const ERealtimeControlSrc src, const ERealtimeControlDst dst, const float amount)
        : m_src(src), m_dst(dst), m_amount(amount) {}
    
    ERealtimeControlSrc m_src = ERealtimeControlSrc::SRC_OFF;
    ERealtimeControlDst m_dst = ERealtimeControlDst::DST_OFF;
    float m_amount = 0.f;
};

constexpr size_t MAX_REALTIME_CONTROLS = 24;

struct BankVoice final
{
    /*
    * Returns false if the control does not exist OR if the amount is 0
    */
    [[nodiscard]] bool GetAmountFromRTControl(ERealtimeControlSrc src, ERealtimeControlDst dst, float& outAmount) const;

    void ReplaceOrAddRTControl(ERealtimeControlSrc src, ERealtimeControlDst dst, float amount);
    void DisableRTControl(ERealtimeControlSrc src, ERealtimeControlDst dst);
    
    std::array<BankRealtimeControl, MAX_REALTIME_CONTROLS> m_realtimeControls{};
    BankLFO m_lfo1 = BankLFO(0., 0ui8, 0., true);
    ADSR_Envelope m_ampEnv{};
    ADSR_Envelope m_filterEnv{};
    BankNoteRange m_keyZone = BankNoteRange(0ui8, 127ui8);
    BankNoteRange m_velocityZone = BankNoteRange(0ui8, 127ui8);
    double m_fineTune = 0.;
    float m_filterQ = 0.f;
    float m_chorusAmount = 0.f;
    float m_chorusWidth = 0.f;
    uint16_t m_filterFrequency = 0ui16;
    int8_t m_transpose = 0i8;
    int8_t m_coarseTune = 0i8;
    int8_t m_volume = 0i8;
    int8_t m_pan = 0i8;
    uint8_t m_originalKey = 0ui8;
    uint16_t m_sampleIndex = 0ui16;
};

struct BankPreset final
{
    explicit BankPreset(const uint16_t index, std::string&& name, std::vector<BankVoice>&& voices)
        : m_presetName(std::move(name)), m_voices(std::move(voices)), m_index(index) {}

    std::string m_presetName;
    std::vector<BankVoice> m_voices{};
    uint16_t m_index = 0ui16;
};

struct BankSequence final
{
    explicit BankSequence(const uint16_t index, std::string&& name, std::vector<char>&& midiData)
        : m_sequenceName(std::move(name)), m_midiData(std::move(midiData)), m_index(index) {}

    std::string m_sequenceName;
    std::vector<char> m_midiData{};
    uint16_t m_index = 0ui16;
};

struct Soundbank final
{
    explicit Soundbank(std::string&& name) : m_bankName(std::move(name)) {}

    void Clear();

    [[nodiscard]] bool IsValid() const { return !m_bankName.empty(); }

    std::string m_bankName;
    std::vector<BankPreset> m_presets{};
    std::vector<BankSample> m_samples{};
    std::vector<BankSequence> m_sequences{};
    uint8_t m_defaultPreset = 255ui8;
};