#pragma once

constexpr auto SAMPLE_LOOP_FLAG = 0x00010000;
constexpr auto SAMPLE_RELEASE_FLAG = 0x00080000;

struct E4Sample final
{
	[[nodiscard]] std::string GetName() const { return std::string(m_name); }
	[[nodiscard]] const std::array<uint32_t, E4BVariables::NUM_SAMPLE_PARAMETERS>& GetParams() const { return m_params; }
	[[nodiscard]] const uint32_t& GetSampleRate() const { return m_sample_rate; }
	[[nodiscard]] const uint32_t& GetFormat() const { return m_format; }
	[[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_sampleIndex); }
private:
	uint16_t m_padding = 0ui16;
	uint16_t m_sampleIndex = 0ui16;
	std::array<char, E4BVariables::E4_MAX_NAME_LEN> m_name{};
	std::array<uint32_t, E4BVariables::NUM_SAMPLE_PARAMETERS> m_params{}; // contains loops etc
	uint32_t m_sample_rate = 0u;
	uint32_t m_format = 0u;
	std::array<uint32_t, E4BVariables::NUM_EXTRA_SAMPLE_PARAMETERS> m_extraParams{}; // always seems to be empty
};

constexpr auto TOTAL_SAMPLE_DATA_READ_SIZE = 92ull;
constexpr auto SAMPLE_DATA_READ_SIZE = 94ull;