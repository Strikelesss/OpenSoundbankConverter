#include "Header/E4B/Data/E4Cord.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

void E4Cord::write(BinaryWriter& writer) const
{
    const auto src(static_cast<uint8_t>(m_src));
    writer.writeType(&src);

    const auto dst(static_cast<uint8_t>(m_dst));
    writer.writeType(&dst);
    
    writer.writeType(&m_amt);
    writer.writeType(&m_possibleRedundant1);
}

void E4Cord::readAtLocation(ReadLocationHandle& readHandle)
{
    uint8_t src;
    readHandle.readType(&src);
    m_src = static_cast<EEOSCordSource>(src);
    
    int8_t dst;
    readHandle.readType(&dst);
    m_dst = static_cast<EEOSCordDest>(dst);
    
    readHandle.readType(&m_amt);
    readHandle.readType(&m_possibleRedundant1);
}