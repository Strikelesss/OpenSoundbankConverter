#include "Header/E4B/Data/E4Zone.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

void E4ZoneNoteData::write(BinaryWriter& writer) const
{
    writer.writeType(&m_low);
    writer.writeType(&m_lowFade);
    writer.writeType(&m_highFade);
    writer.writeType(&m_high);
}

void E4ZoneNoteData::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_low);
    readHandle.readType(&m_lowFade);
    readHandle.readType(&m_highFade);
    readHandle.readType(&m_high);
}

void E4Zone::write(BinaryWriter& writer) const
{
    m_keyData.write(writer);
    m_velData.write(writer);
    writer.writeType(&m_sampleIndex);
    writer.writeType(&m_possibleRedundant1);
    writer.writeType(&m_fineTune);
    writer.writeType(&m_originalKey);
    writer.writeType(&m_volume);
    writer.writeType(&m_pan);
    writer.writeType(m_possibleRedundant2.data(), m_possibleRedundant2.size());
}

void E4Zone::readAtLocation(ReadLocationHandle& readHandle)
{
    m_keyData.readAtLocation(readHandle);
    m_velData.readAtLocation(readHandle);
    readHandle.readType(&m_sampleIndex);
    readHandle.readType(&m_possibleRedundant1);
    readHandle.readType(&m_fineTune);
    readHandle.readType(&m_originalKey);
    readHandle.readType(&m_volume);
    readHandle.readType(&m_pan);
    readHandle.readType(m_possibleRedundant2.data(), m_possibleRedundant2.size());
}

double E4Zone::GetFineTune() const
{
    return E4VoiceHelpers::ConvertByteToFineTune(m_fineTune);
}