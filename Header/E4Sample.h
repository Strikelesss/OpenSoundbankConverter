#pragma once
#include "E4BVariables.h"

namespace E4SampleVariables
{
	constexpr auto EOS_MONO_SAMPLE = 0x00300001;
	constexpr auto EOS_STEREO_SAMPLE = 0x00700001;
	constexpr auto EOS_STEREO_SAMPLE_2 = 0x00700000;
	constexpr auto SAMPLE_LOOP_FLAG = 0x00010000;
	constexpr auto SAMPLE_RELEASE_FLAG = 0x00080000;

	constexpr auto SAMPLE_LOOP_START_PARAM_INDEX = 5;
	constexpr auto SAMPLE_LOOP_END_PARAM_INDEX = 7;
}

/*
	// Params:
	
	// 0 = ???
	// 1 = start of left channel (TOTAL_SAMPLE_DATA_READ_SIZE)
	// 2 = start of right channel (0 if mono)
	// 3 = last sample of left channel (lastLoc - wavStart - 2 + TOTAL_SAMPLE_DATA_READ_SIZE)
	// 4 = last sample of right channel (3 - TOTAL_SAMPLE_DATA_READ_SIZE)
	// 5 = loop start (* 2 + TOTAL_SAMPLE_DATA_READ_SIZE)
	// 6 = 5 - TOTAL_SAMPLE_DATA_READ_SIZE
	// 7 = loop end (* 2 + TOTAL_SAMPLE_DATA_READ_SIZE)
	// 8 = 7 - TOTAL_SAMPLE_DATA_READ_SIZE
 */

struct E4Sample final
{
	[[nodiscard]] std::string GetName() const { return std::string(m_name); }
	[[nodiscard]] const std::array<uint32_t, E4BVariables::EOS_NUM_SAMPLE_PARAMETERS>& GetParams() const { return m_params; }
	[[nodiscard]] uint32_t GetSampleRate() const { return m_sampleRate; }
	[[nodiscard]] uint32_t GetFormat() const { return m_format; }
	[[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_sampleIndex); }
private:
	uint16_t m_padding = 0ui16;
	uint16_t m_sampleIndex = 0ui16; // requires byteswap
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
	std::array<uint32_t, E4BVariables::EOS_NUM_SAMPLE_PARAMETERS> m_params{}; // contains loops etc
	uint32_t m_sampleRate = 0u;
	uint32_t m_format = 0u;

	//std::array<uint32_t, E4BVariables::NUM_EXTRA_SAMPLE_PARAMETERS> m_extraParams{}; // always seems to be empty, safe to ignore
};

constexpr auto TOTAL_SAMPLE_DATA_READ_SIZE = 92ull;
constexpr auto SAMPLE_DATA_READ_SIZE = 64ull;