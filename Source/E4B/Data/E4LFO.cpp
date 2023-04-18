#include "Header/E4B/Data/E4LFO.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

E4LFO::E4LFO(const double rate, const uint8_t shape, const double delay, const bool keySync) : m_rate(E4VoiceHelpers::GetByteFromLFORate(rate)),
    m_shape(shape), m_delay(E4VoiceHelpers::GetByteFromLFODelay(delay)), m_keySync(!keySync) {}

void E4LFO::write(BinaryWriter& writer) const
{
    writer.writeType(&m_rate);
    writer.writeType(&m_shape);
    writer.writeType(&m_delay);
    writer.writeType(&m_variation);
    writer.writeType(&m_keySync);
    writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
}

void E4LFO::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_rate);
    readHandle.readType(&m_shape);
    readHandle.readType(&m_delay);
    readHandle.readType(&m_variation);
    readHandle.readType(&m_keySync);
    readHandle.readType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
}

double E4LFO::GetRate() const
{
    return E4VoiceHelpers::GetLFORateFromByte(m_rate);
}

double E4LFO::GetDelay() const
{
    return E4VoiceHelpers::GetLFODelayFromByte(m_delay);
}