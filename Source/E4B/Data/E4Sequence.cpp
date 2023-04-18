#include "Header/E4B/Data/E4Sequence.h"
#include "Header/E4B/Helpers/E4BVariables.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/IO/E4BReader.h"

E4Sequence::E4Sequence(const E4TOCChunk& chunk, BinaryReader& reader)
{
    ReadLocationHandle readHandle(reader, chunk.GetStartOffset() + sizeof(E4DataChunk));
    readAtLocation(readHandle);
    
    const size_t midiSize(chunk.GetLength() + sizeof(uint16_t) - SEQUENCE_DATA_READ_SIZE);
    m_midiData.resize(midiSize);
    readHandle.readType(m_midiData.data(), sizeof(char) * midiSize);
}

void E4Sequence::write(BinaryWriter& writer) const
{
    writer.writeType(&m_seqIndex);
    writer.writeType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
}

void E4Sequence::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_seqIndex);
    readHandle.readType(m_name.data(), sizeof(char) * E4BVariables::EOS_E4_MAX_NAME_LEN);
}