#include "Header/E4B/Data/E4MIDIChannel.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

void E4MIDIChannel::write(BinaryWriter& writer) const
{
    writer.writeType(&m_volume);
    writer.writeType(&m_pan);
    writer.writeType(m_possibleRedundant1.data(), sizeof(uint8_t) * m_possibleRedundant1.size());
    writer.writeType(&m_aux);
    writer.writeType(m_controllers.data(), sizeof(uint8_t) * m_controllers.size());
    writer.writeType(m_possibleRedundant2.data(), sizeof(uint8_t) * m_possibleRedundant2.size());
    writer.writeType(&m_presetNum);
}

void E4MIDIChannel::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_volume);
    readHandle.readType(&m_pan);
    readHandle.readType(m_possibleRedundant1.data(), sizeof(uint8_t) * m_possibleRedundant1.size());
    readHandle.readType(&m_aux);
    readHandle.readType(m_controllers.data(), sizeof(uint8_t) * m_controllers.size());
    readHandle.readType(m_possibleRedundant2.data(), sizeof(uint8_t) * m_possibleRedundant2.size());
    readHandle.readType(&m_presetNum);
}