#include "Header/E4B/Data/E3Sample.h"
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Helpers/E4BHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/IO/E4BReader.h"

E3SampleParams::E3SampleParams(const uint32_t sampleSize, const uint32_t loopStart, const uint32_t loopEnd)
    : m_lastSampleLeftChannel((sampleSize * 2u) - 2u + E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE),
    m_lastSampleRightChannel((sampleSize * 2u) - 2u), m_loopStart(loopStart * 2u + static_cast<uint32_t>(E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE)),
    m_loopStart2(m_loopStart - E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE), m_loopEnd(loopEnd * 2u + static_cast<uint32_t>(E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE)),
    m_loopEnd2(m_loopEnd - E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE) {}

void E3SampleParams::write(BinaryWriter& writer) const
{
    writer.writeType(&m_unknown);
    writer.writeType(&m_leftChannelStart);
    writer.writeType(&m_rightChannelStart);
    writer.writeType(&m_lastSampleLeftChannel);
    writer.writeType(&m_lastSampleRightChannel);
    writer.writeType(&m_loopStart);
    writer.writeType(&m_loopStart2);
    writer.writeType(&m_loopEnd);
    writer.writeType(&m_loopEnd2);
}

void E3SampleParams::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_unknown);
    readHandle.readType(&m_leftChannelStart);
    readHandle.readType(&m_rightChannelStart);
    readHandle.readType(&m_lastSampleLeftChannel);
    readHandle.readType(&m_lastSampleRightChannel);
    readHandle.readType(&m_loopStart);
    readHandle.readType(&m_loopStart2);
    readHandle.readType(&m_loopEnd);
    readHandle.readType(&m_loopEnd2);
}

E3Sample::E3Sample(const E4TOCChunk& chunk, BinaryReader& reader)
{
    const uint64_t offset(chunk.GetStartOffset() + sizeof(E4DataChunk));
    ReadLocationHandle readHandle(reader, offset);

    readHandle.readType(&m_sampleIndex);
    readHandle.readType(m_name.data(),sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    m_params.readAtLocation(readHandle);
    readHandle.readType(&m_sampleRate);
    readHandle.readType(&m_format);
    readHandle.readType(m_extraParams.data(), sizeof(uint32_t) * E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS);
    
    const size_t wavSize(chunk.GetLength() + sizeof(uint16_t) - E3SampleVariables::SAMPLE_DATA_READ_SIZE);
    m_sampleData.resize(wavSize / sizeof(int16_t));
    readHandle.readType(m_sampleData.data(), sizeof(int16_t) * m_sampleData.size());
}

E3Sample::E3Sample(const BankSample& sample) : m_sampleIndex(_byteswap_ushort(sample.m_index + 1ui16)), m_name(E4BHelpers::ConvertToE4Name(sample.m_sampleName)),
    m_params(static_cast<uint32_t>(sample.m_sampleData.size()), sample.m_loopStart, sample.m_loopEnd), m_sampleRate(sample.m_sampleRate), m_sampleData(sample.m_sampleData)
{
    if(sample.m_channels == 1u)
    {
        m_format = E3SampleVariables::EOS_MONO_SAMPLE;
    }
    else if(sample.m_channels == 2u)
    {
        m_format = E3SampleVariables::EOS_STEREO_SAMPLE;
    }
    else
    {
        // Should not be hitting this
        assert(sample.m_channels <= 2u);
    }
    
    if(sample.m_isLooping) { m_format |= E3SampleVariables::SAMPLE_LOOP_FLAG; }
    if(sample.m_isLoopReleasing) { m_format |= E3SampleVariables::SAMPLE_RELEASE_FLAG; }
}

void E3Sample::write(BinaryWriter& writer) const
{
    writer.writeType(&m_sampleIndex);
    writer.writeType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    m_params.write(writer);
    writer.writeType(&m_sampleRate);
    writer.writeType(&m_format);
    writer.writeType(m_extraParams.data(), sizeof(uint32_t) * E4BVariables::EOS_NUM_EXTRA_SAMPLE_PARAMETERS);
    writer.writeType(m_sampleData.data(), sizeof(uint16_t) * m_sampleData.size());
}

uint32_t E3Sample::GetNumChannels() const
{
    if ((m_format & E3SampleVariables::EOS_STEREO_SAMPLE) == E3SampleVariables::EOS_STEREO_SAMPLE || 
        (m_format & E3SampleVariables::EOS_STEREO_SAMPLE_2) == E3SampleVariables::EOS_STEREO_SAMPLE_2) { return 2u; }

    return 1u;
}

uint32_t E3Sample::GetLoopStart() const
{
    if(IsLooping())
    {
        if(m_params.m_loopStart > E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE)
        {
            return (m_params.m_loopStart - E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
        }
    }

    return 0u;
}

uint32_t E3Sample::GetLoopEnd() const
{
    if(IsLooping())
    {
        if(m_params.m_loopEnd > E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE)
        {
            return (m_params.m_loopEnd - E3SampleVariables::TOTAL_SAMPLE_DATA_READ_SIZE) / 2u;
        } 
    }

    return 0u;
}

bool E3Sample::IsLooping() const
{
    return (m_format & E3SampleVariables::SAMPLE_LOOP_FLAG) == E3SampleVariables::SAMPLE_LOOP_FLAG;
}

bool E3Sample::IsLoopReleasing() const
{
    return (m_format & E3SampleVariables::SAMPLE_RELEASE_FLAG) == E3SampleVariables::SAMPLE_RELEASE_FLAG;
}
