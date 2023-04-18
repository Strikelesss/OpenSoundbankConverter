#include "Header/E4B/Data/E4Preset.h"
#include "Header/Data/Soundbank.h"
#include "Header/E4B/Helpers/E4BHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/IO/E4BReader.h"

E4Preset::E4Preset(const E4TOCChunk& chunk, BinaryReader& reader)
{
    ReadLocationHandle readHandle(reader, chunk.GetStartOffset() + sizeof(E4DataChunk));
    readAtLocation(readHandle);

    const auto numVoices(GetNumVoices());
    if (numVoices > 0ui16)
    {
        uint16_t voiceOffset(0ui16);

        for (uint16_t j(0ui16); j < numVoices; ++j)
        {
            E4Voice voice(chunk, GetDataSize(), voiceOffset, reader);
            voiceOffset += voice.GetVoiceDataSize();
            m_voices.emplace_back(std::move(voice));
        }
    }
}

E4Preset::E4Preset(const BankPreset& preset) : m_index(_byteswap_ushort(preset.m_index)),
    m_name(E4BHelpers::ConvertToE4Name(preset.m_presetName)), m_dataSize(_byteswap_ushort(TOTAL_PRESET_DATA_SIZE)),
    m_numVoices(_byteswap_ushort(static_cast<uint16_t>(preset.m_voices.size()))),
    m_voices(E4BHelpers::GetE4VoicesFromBankVoices(preset.m_voices)) {}

void E4Preset::write(BinaryWriter& writer) const
{
    writer.writeType(&m_index);
    writer.writeType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    writer.writeType(&m_dataSize);
    writer.writeType(&m_numVoices);
    writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
    writer.writeType(&m_transpose);
    writer.writeType(&m_volume);
    writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    writer.writeType(m_possibleRedundant3.data(), sizeof(int8_t) * m_possibleRedundant3.size());
    writer.writeType(m_midiControllers.data(), sizeof(uint8_t) * m_midiControllers.size());
    writer.writeType(m_possibleRedundant4.data(), sizeof(uint8_t) * m_possibleRedundant4.size());
    
    for(const auto& voice : m_voices)
    {
        voice.write(writer);
    }
}

void E4Preset::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_index);
    readHandle.readType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    readHandle.readType(&m_dataSize);
    readHandle.readType(&m_numVoices);
    readHandle.readType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
    readHandle.readType(&m_transpose);
    readHandle.readType(&m_volume);
    readHandle.readType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    readHandle.readType(m_possibleRedundant3.data(), sizeof(int8_t) * m_possibleRedundant3.size());
    readHandle.readType(m_midiControllers.data(), sizeof(uint8_t) * m_midiControllers.size());
    readHandle.readType(m_possibleRedundant4.data(), sizeof(uint8_t) * m_possibleRedundant4.size());
}