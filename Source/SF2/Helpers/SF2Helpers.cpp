#include "Header/SF2/Helpers/SF2Helpers.h"
#include "Header/Logger.h"
#include <cassert>
#include <cmath>

//
// These are NOT correct but will work until we can determine how each sound in the SF2
// TODO: correct these values

int16_t SF2Helpers::filterFreqPercentToCents(const float filterFreq)
{
    if (filterFreq > MAX_FILTER_FREQ_HZ_CORDS || filterFreq < -MAX_FILTER_FREQ_HZ_CORDS)
    {
        Logger::LogMessage("Invalid filter frequency!");
        assert(false);
        return 0i16;
    }
    return static_cast<int16_t>(MAX_FILTER_FREQ_HZ_CORDS * filterFreq / 100.f);
}

float SF2Helpers::centsToFilterFreqPercent(const int16_t cents)
{
    return static_cast<float>(cents) * 100.f / MAX_FILTER_FREQ_HZ_CORDS;
}

//

int16_t SF2Helpers::secToTimecent(const double sec)
{
    return static_cast<int16_t>(std::lround(std::log(sec) / std::log(2) * 1200.));
}

int16_t SF2Helpers::convert_dB_to_cB(const float db)
{
    return static_cast<int16_t>(std::roundf(db * 10.f));
}

double SF2Helpers::centsToHertz(const int16_t cents)
{
    return 8.176 * std::pow(2., static_cast<double>(cents) / 1200.);
}

int16_t SF2Helpers::hertzToCents(const double hz)
{
    return static_cast<int16_t>(std::round(std::log(hz / 8.176) / std::log(2.) * 1200.));
}

void SF2Helpers::InterleaveSamples(const int16_t* leftChannel, const int16_t* rightChannel, int16_t* outStereo,
    const size_t sampleNum)
{
    for (size_t i(0); i < sampleNum; ++i)
    {
        outStereo[i * 2] = leftChannel[i];
        outStereo[i * 2 + 1] = rightChannel[i];
    }
}