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
			std::ofstream of(m_writeFile.data(), std::ios::binary);
			of.write(m_writeDataVector.data(), static_cast<std::streamsize>(m_writeDataVector.size()));
		}

		//m_writeDataVector.clear();
		//m_writeData = nullptr;
		//m_bytesWritten = 0;
		return true;
	}

	return false;
}

bool BinaryWriter::CanFitWrite(const size_t dataSize) const
{
	return m_writeDataVector.size() - m_bytesWritten >= dataSize;
}