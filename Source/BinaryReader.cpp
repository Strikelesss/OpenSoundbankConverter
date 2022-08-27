#include "Header/BinaryReader.h"
#include <fstream>

bool BinaryReader::readFile(const std::filesystem::path& file)
{
	m_filePath = file;

	std::ifstream ifs(file.c_str(), std::ios::binary);
	if (ifs.peek() == std::ifstream::traits_type::eof()) { return false; }

	ifs.seekg(0, std::ifstream::end);
	m_readDataVector.resize(ifs.tellg());
	ifs.seekg(0, std::ifstream::beg);
	ifs.read(m_readDataVector.data(), static_cast<std::streamsize>(m_readDataVector.size()));
	m_readData = m_readDataVector.data();
	return true;
}