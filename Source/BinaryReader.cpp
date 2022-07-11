#include "Header/BinaryReader.h"
#include <cassert>
#include <fstream>

bool BinaryReader::readFile(const std::filesystem::path& file)
{
	m_filePath = file;

	std::ifstream fs(file.c_str(), std::ios::binary);
	if (fs.peek() == std::ifstream::traits_type::eof())
	{
		//std::printf("Binary file %s contains no data!", file.filename().string().c_str());
		assert(fs.peek() != std::ifstream::traits_type::eof());
		return false;
	}

	fs.seekg(0, std::ifstream::end);
	m_readDataVector.resize(fs.tellg());
	fs.seekg(0, std::ifstream::beg);
	fs.read(m_readDataVector.data(), static_cast<std::streamsize>(m_readDataVector.size()));
	m_readData = m_readDataVector.data();
	return true;
}