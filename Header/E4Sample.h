#pragma once

constexpr auto SAMPLE_LOOP_FLAG = 0x00010000;
constexpr auto SAMPLE_RELEASE_FLAG = 0x00080000;
constexpr auto SAMPLE_LOOP_OFFSET = 92u;

struct E4Sample final
{
	std::array<char, E4BVariables::NAME_SIZE> m_name{};
	std::array<uint32_t, E4BVariables::NUM_SAMPLE_PARAMETERS> m_params{};
	uint32_t m_sample_rate = 0u;
	uint32_t m_format = 0u;
	std::array<uint32_t, E4BVariables::NUM_EXTRA_SAMPLE_PARAMETERS> m_extraParams{};
	uint32_t m_padding = 0u;
};

constexpr auto SAMPLE_DATA_READ_SIZE = 92ull;