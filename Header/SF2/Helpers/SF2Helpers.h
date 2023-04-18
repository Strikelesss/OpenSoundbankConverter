#pragma once
#include <cstdint>

/*
* Converter specific info
*/

// kUnused1 = LFO1 ~ -> Amp Pan
// kUnused2 = Positive or negative attenuation
// kUnused3 = LFO1 shape
// kUnused4 = LFO1 Key Sync
// kUnused5 = Chorus Width

namespace SF2Helpers
{
    constexpr auto SF2_MAX_NAME_LEN = 20u;
    constexpr auto BASE_CENT_VALUE = 6900i16;
    constexpr auto SF2_FILTER_MIN_FREQ = 1500i16;
    constexpr auto SF2_FILTER_MAX_FREQ = 13500i16;
    constexpr auto MIN_MAX_LFO1_TO_VOLUME = 15.f;
    constexpr auto MAX_FILTER_FREQ_HZ_CORDS(12000.f);
    constexpr auto MAX_SUSTAIN_VOL_ENV = 144.f;

    [[nodiscard]] int16_t filterFreqPercentToCents(float filterFreq);
    [[nodiscard]] float centsToFilterFreqPercent(int16_t cents);
    [[nodiscard]] int16_t secToTimecent(double sec);
    
    [[nodiscard]] constexpr float convert_cB_to_dB(const float cb) { return cb / 10.f; }
    [[nodiscard]] int16_t convert_dB_to_cB(float db);

    [[nodiscard]] double centsToHertz(int16_t cents);
    [[nodiscard]] int16_t hertzToCents(double hz);

    template<typename T>
    [[nodiscard]] constexpr int16_t valueToRelativePercent(const T val) { return static_cast<int16_t>(val * T(10)); }

    [[nodiscard]] constexpr float relPercentToValue(const float val) { return val / 10.f; }

    void InterleaveSamples(const int16_t* leftChannel, const int16_t* rightChannel, int16_t* outStereo, size_t sampleNum);
};