#pragma once
#include <filesystem>
#include <cassert>

struct BinaryWriter final
{
	explicit BinaryWriter(const std::filesystem::path& file) : m_writeFile(file.native()), m_writeData(m_writeDataVector.data()) {}

	[[nodiscard]] size_t GetWritePos() const { return m_bytesWritten; }
	[[nodiscard]] bool finishWriting();

    void writeNull(const size_t nullLength)
    {
        assert(nullLength > 0);
        if(nullLength > 0)
        {
            m_writeData += nullLength;
            m_bytesWritten += nullLength;
        }
    }
    
	template<typename T>
	void writeType(const T* data, const size_t size = sizeof(T))
	{
        static_assert(std::is_fundamental_v<T>);
		if(data == nullptr) { assert(data != nullptr); return; }
		
		if (!CanFitWrite(size))
		{
			m_writeDataVector.resize(m_bytesWritten + size);
			m_writeData = m_writeDataVector.data();
			m_writeData += m_bytesWritten;
		}
        
		std::memcpy(m_writeData, data, size);
		m_writeData += size;
		m_bytesWritten += size;
	}

	template<typename T>
	void writeTypeAtLocation(const T* data, const size_t location, const size_t size = sizeof(T))
	{
        static_assert(std::is_fundamental_v<T>);
		if(data == nullptr || location > m_writeDataVector.size()) { assert(data != nullptr && location <= m_writeDataVector.size()); return; }
        
		if(location + size > m_writeDataVector.size()) { assert(location + size <= m_writeDataVector.size()); return; }

		std::memcpy(std::next(m_writeDataVector.data(), location), data, size);
	}

private:
	[[nodiscard]] bool CanFitWrite(size_t dataSize) const;
	std::vector<char> m_writeDataVector = std::vector<char>(1000); // Start out at 1000 to avoid extra resizes
	std::wstring_view m_writeFile;
	char* m_writeData = nullptr;
	size_t m_bytesWritten = 0;
};