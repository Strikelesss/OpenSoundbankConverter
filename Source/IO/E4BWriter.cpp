#include "Header/IO/E4BWriter.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/Logger.h"
#include "Header/MathFunctions.h"
#include "Header/E4B/Data/E4Preset.h"
#include "Header/E4B/Data/E3Sample.h"
#include "Header/E4B/Helpers/E4BHelpers.h"
#include "Header/IO/E4BReader.h"
#include "Header/E4B/Data/EMSt.h"

void E4BWriter::BeginWriting(BinaryWriter& writer)
{
    m_beganWriting = true;
    
    // Write the beginning FORM tag
    writer.writeType(E4BVariables::EOS_FORM_TAG.data(), sizeof(char) * E4BVariables::EOS_FORM_TAG.length());

    // Write a null FORM size, this is temporary until we've input everything.
    writer.writeNull(sizeof(uint32_t));

    // Write E4B0 tag
    writer.writeType(E4BVariables::EOS_E4_FORMAT_TAG.data(), sizeof(char) * E4BVariables::EOS_E4_FORMAT_TAG.length());

    m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_E4_FORMAT_TAG.length());

    // Write TOC1 tag
    writer.writeType(E4BVariables::EOS_TOC_TAG.data(), sizeof(char) * E4BVariables::EOS_TOC_TAG.length());

    m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_TOC_TAG.length());

    // Write a null TOC length, this is temporary until we've input everything.
    writer.writeNull(sizeof(uint32_t));

    m_totalFORMSize += static_cast<uint32_t>(sizeof(uint32_t));
}

void E4BWriter::EndWriting(BinaryWriter& writer)
{
    // Write TOC:
    WriteTOC(writer);
    
    // Write the total FORM size now that we've input everything into the file.
    const uint32_t byteswapTotalFormSize(MathFunctions::byteswapUINT32(m_totalFORMSize));
    writer.writeTypeAtLocation(&byteswapTotalFormSize, E4BVariables::EOS_FORM_TAG.length());

    // Write the total indexing size now that we've input everything into the file.
    const uint32_t byteswapTotalIndexingSize(MathFunctions::byteswapUINT32(m_totalIndexingSize));
    writer.writeTypeAtLocation(&byteswapTotalIndexingSize, E4BVariables::EOS_FORM_TAG.length() + sizeof(uint32_t) +
        E4BVariables::EOS_E4_FORMAT_TAG.length() + E4BVariables::EOS_TOC_TAG.length());
    
    if(writer.finishWriting()) { Logger::LogMessage("Successfully wrote E4B file!"); }
    else { Logger::LogMessage("Failed to write E4B file!"); }

    m_beganWriting = false;
}

std::string E4BWriter::ConvertNameToEmuName(const std::string_view& name) const
{
    std::string str(std::begin(name), std::ranges::find(name, '\0'));
    if (name.length() > E4BVariables::EOS_E4_MAX_NAME_LEN) { str.resize(E4BVariables::EOS_E4_MAX_NAME_LEN); }

    if (str.length() < E4BVariables::EOS_E4_MAX_NAME_LEN)
    {
        for (size_t i(str.length()); i < E4BVariables::EOS_E4_MAX_NAME_LEN; ++i)
        {
            str.append(" ");
        }
    }

    return str;
}

void E4BWriter::WriteTOC(BinaryWriter& writer)
{
    std::vector<size_t> presetTOCChunkLocations{};
    for(const auto& preset : m_presets)
    {
        presetTOCChunkLocations.emplace_back(writer.GetWritePos() + 8);
        
        const uint32_t presetDataLength(TOTAL_PRESET_DATA_SIZE + (static_cast<uint32_t>(preset.m_voices.size()) * VOICE_1_ZONE_DATA_SIZE));
        E4TOCChunk E4P1Chunk(E4BHelpers::ConvertToE4ChunkName(E4BVariables::EOS_E4_PRESET_TAG), presetDataLength, 0u);
        E4P1Chunk.write(writer);

        const uint16_t presetIndex(_byteswap_ushort(preset.m_index));
        writer.writeType(&presetIndex);

        const auto presetName(E4BHelpers::ConvertToE4Name(preset.m_presetName));
        writer.writeType(presetName.data(), sizeof(char) * presetName.size());

        writer.writeNull(sizeof(uint16_t));
        
        m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_TOTAL_LEN);
        m_totalIndexingSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_TOTAL_LEN);
    }

    std::vector<size_t> sampleTOCChunkLocations{};
    for(const auto& sample : m_samples)
    {
        sampleTOCChunkLocations.emplace_back(writer.GetWritePos() + 8);
        
        const uint32_t sampleDataLength(static_cast<uint32_t>(E3SampleVariables::SAMPLE_DATA_READ_SIZE +
            sizeof(uint16_t) * sample.m_sampleData.size()) - sizeof(uint16_t));
        
        E4TOCChunk E3S1Chunk(E4BHelpers::ConvertToE4ChunkName(E4BVariables::EOS_E3_SAMPLE_TAG), sampleDataLength, 0u);
        E3S1Chunk.write(writer);

        const uint16_t sampleIndex(_byteswap_ushort(sample.m_index + 1ui16));
        writer.writeType(&sampleIndex);

        const auto sampleName(E4BHelpers::ConvertToE4Name(sample.m_sampleName));
        writer.writeType(sampleName.data(), sizeof(char) * sampleName.size());

        writer.writeNull(sizeof(uint16_t));
        
        m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_TOTAL_LEN);
        m_totalIndexingSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_TOTAL_LEN);
    }
    
    size_t index(0);
    for(const auto& preset : m_presets)
    {
        const uint32_t writePos(MathFunctions::byteswapUINT32(static_cast<uint32_t>(writer.GetWritePos())));
        writer.writeTypeAtLocation(&writePos, presetTOCChunkLocations[index]);
        
        const uint32_t presetDataLength((TOTAL_PRESET_DATA_SIZE + (static_cast<uint32_t>(preset.m_voices.size()) * VOICE_1_ZONE_DATA_SIZE)) + sizeof(uint16_t));
        E4DataChunk E4P1Chunk(E4BHelpers::ConvertToE4ChunkName(E4BVariables::EOS_E4_PRESET_TAG), presetDataLength);
        E4P1Chunk.write(writer);
        
        m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_SIZE);
        
        E4Preset e4Preset(preset);
        e4Preset.write(writer);

        m_totalFORMSize += presetDataLength;
        ++index;
    }

    index = 0;
    
    for(const auto& sample : m_samples)
    {
        const uint32_t writePos(MathFunctions::byteswapUINT32(static_cast<uint32_t>(writer.GetWritePos())));
        writer.writeTypeAtLocation(&writePos, sampleTOCChunkLocations[index]);

        const uint32_t sampleDataLength(static_cast<uint32_t>(E3SampleVariables::SAMPLE_DATA_READ_SIZE + sizeof(uint16_t) * sample.m_sampleData.size()));
        E4DataChunk E3S1Chunk(E4BHelpers::ConvertToE4ChunkName(E4BVariables::EOS_E3_SAMPLE_TAG), sampleDataLength);
        E3S1Chunk.write(writer);
        
        m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_SIZE);
        
        E3Sample e3Sample(sample);
        e3Sample.write(writer);

        m_totalFORMSize += sampleDataLength;
        ++index;
    }

    E4DataChunk EMStChunk(E4BHelpers::ConvertToE4ChunkName(E4BVariables::EOS_EMSt_TAG), TOTAL_EMST_DATA_SIZE);
    EMStChunk.write(writer);
    
    m_totalFORMSize += static_cast<uint32_t>(E4BVariables::EOS_CHUNK_SIZE);

    E4EMSt emst(0ui8);
    emst.write(writer);

    m_totalFORMSize += TOTAL_EMST_DATA_SIZE;
}