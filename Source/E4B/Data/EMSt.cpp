#include "Header/E4B/Data/EMSt.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

void E4EMSt::write(BinaryWriter& writer) const
{
    writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
    writer.writeType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    writer.writeType(&m_currentPreset);
    
    for(auto& midiChannel : m_midiChannels)
    {
        midiChannel.write(writer);
    }

    writer.writeType(m_possibleRedundant3.data(), sizeof(uint8_t) * m_possibleRedundant3.size());
    writer.writeType(&m_tempo);
    writer.writeType(m_possibleRedundant4.data(), sizeof(uint8_t) * m_possibleRedundant4.size());
}

void E4EMSt::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
    readHandle.readType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
    readHandle.readType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size());
    readHandle.readType(&m_currentPreset);
    
    for(auto& midiChannel : m_midiChannels)
    {
        midiChannel.readAtLocation(readHandle);
    }

    readHandle.readType(m_possibleRedundant3.data(), sizeof(uint8_t) * m_possibleRedundant3.size());
    readHandle.readType(&m_tempo);
    readHandle.readType(m_possibleRedundant4.data(), sizeof(uint8_t) * m_possibleRedundant4.size());
}