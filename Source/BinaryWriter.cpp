#include "Header/BinaryWriter.h"
#include <fstream>

bool BinaryWriter::finishWriting()
{
	if (!m_writeDataVector.empty())
	{
		if (m_writeDataVector.size() > m_bytesWritten)
		{
			m_writeDataVector.resize(m_bytesWritten);
		}

		if(!m_writeFile.empty())
		{
			std::ofstream ofs(m_writeFile.data(), std::ios::binary);
			ofs.write(m_writeDataVector.data(), static_cast<std::streamsize>(m_writeDataVector.size()));
		}

		return true;
	}

	return false;
}

bool BinaryWriter::CanFitWrite(const size_t dataSize) const
{
	return m_writeDataVector.size() - m_bytesWritten >= dataSize;
}