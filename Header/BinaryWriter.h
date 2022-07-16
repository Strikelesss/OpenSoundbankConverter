#pragma once
#include <filesystem>

struct BinaryWriter final
{
	explicit BinaryWriter(const std::filesystem::path& file) : m_writeFile(file.native()), m_writeData(m_writeDataVector.data()) {}

	[[nodiscard]] bool finishWriting();

	template<typename T>
	[[nodiscard]] bool writeType(T* data, const size_t& customSize = 0)
	{
		if(data == nullptr) { return false; }
		
		if ((customSize == 0 && !CanFitWrite(sizeof(T)))
			|| !CanFitWrite(customSize))
		{
			const auto size(customSize == 0 ? sizeof(T) : customSize);
			m_writeDataVector.resize(m_bytesWritten + size);
			m_writeData = m_writeDataVector.data();
			m_writeData += m_bytesWritten;
		}

		if constexpr(std::is_same_v<const char*, decltype(data)>)
		{
			if (static_cast<const char*>(data)[0] == '\0') { return false; }
		}

		const auto size(customSize == 0 ? sizeof(T) : customSize);
		std::memcpy(m_writeData, data, size);
		m_writeData += size;
		m_bytesWritten += size;
		return true;
	}

private:
	[[nodiscard]] bool CanFitWrite(size_t dataSize) const;
	std::vector<char> m_writeDataVector = std::vector<char>(1000); // Start out at 1000 to avoid extra resizes
	std::wstring_view m_writeFile;
	char* m_writeData = nullptr;
	size_t m_bytesWritten = 0;
};
