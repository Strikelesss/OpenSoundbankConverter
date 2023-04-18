#pragma once
#include "Header/E4B/Helpers/E4BVariables.h"
#include <vector>

struct ReadLocationHandle;
struct BinaryWriter;
struct BankSample;
struct E4TOCChunk;
struct BinaryReader;

namespace E3SampleVariables
{
	constexpr auto EOS_MONO_SAMPLE = 0x00300001;
	constexpr auto EOS_STEREO_SAMPLE = 0x00700001;
	constexpr auto EOS_STEREO_SAMPLE_2 = 0x00700000;
	constexpr auto SAMPLE_LOOP_FLAG = 0x00010000;
	constexpr auto SAMPLE_RELEASE_FLAG = 0x00080000;
    
    constexpr auto TOTAL_SAMPLE_DATA_READ_SIZE = 92ull;
    constexpr auto SAMPLE_DATA_READ_SIZE = 94ull;
}

struct E3SampleParams final
{
    explicit E3SampleParams(uint32_t sampleSize, uint32_t loopStart, uint32_t loopEnd);

    void write(BinaryWriter& writer) const;
    void readAtLocation(ReadLocationHandle& readHandle);
    
    uint32_t m_unknown = 0u;
    uint32_t m_leftChannelStart = static_cast<uint32_t>(E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE);
    uint32_t m_rightChannelStart = 0u; // = 0 if mono
    uint32_t m_lastSampleLeftChannel = 0u; // = sample size - 2 + TOTAL_SAMPLE_DATA_READ_SIZE
    uint32_t m_lastSampleRightChannel = 0u; // = sample size - 2
    uint32_t m_loopStart = 0u; // multiplied by 2 and adding TOTAL_SAMPLE_DATA_READ_SIZE
    uint32_t m_loopStart2 = 0u; // = loopStart - TOTAL_SAMPLE_DATA_READ_SIZE
    uint32_t m_loopEnd = 0u; // multiplied by 2 and adding TOTAL_SAMPLE_DATA_READ_SIZE
    uint32_t m_loopEnd2 = 0u; // = loopEnd - TOTAL_SAMPLE_DATA_READ_SIZE
};

struct E3Sample final
{
    explicit E3Sample(const E4TOCChunk& chunk, BinaryReader& reader);
    explicit E3Sample(const BankSample& sample);

    void write(BinaryWriter& writer) const;
    
	[[nodiscard]] std::string_view GetName() const { return {m_name.data(), m_name.size()}; }
	[[nodiscard]] const E3SampleParams& GetParams() const { return m_params; }
    [[nodiscard]] std::vector<int16_t>& GetData() { return m_sampleData; }
	[[nodiscard]] uint32_t GetSampleRate() const { return m_sampleRate; }
	[[nodiscard]] uint32_t GetFormat() const { return m_format; }
	[[nodiscard]] uint16_t GetIndex() const { return _byteswap_ushort(m_sampleIndex); }
    [[nodiscard]] uint32_t GetNumChannels() const;
    [[nodiscard]] uint32_t GetLoopStart() const;
    [[nodiscard]] uint32_t GetLoopEnd() const;
    [[nodiscard]] bool IsLooping() const;
    [[nodiscard]] bool IsLoopReleasing() const;
    
private:

    /*
     * Read data (follows SAMPLE_DATA_READ_SIZE)
     */
    
	uint16_t m_sampleIndex = 0ui16; // requires byteswap
	std::array<char, E4BVariables::EOS_E4_MAX_NAME_LEN> m_name{};
    
    E3SampleParams m_params = E3SampleParams(0u, 0u, 0u);
    
	uint32_t m_sampleRate = 0u;
	uint32_t m_format = 0u;

	std::array<uint32_t, E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS> m_extraParams{}; // Always seems to be empty

    /*
     * Allocated data
     */
    
    std::vector<int16_t> m_sampleData{};
};