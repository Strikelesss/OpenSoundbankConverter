#pragma once
#include <filesystem>

struct BinaryReader final
{
	BinaryReader() = default;
	bool readFile(const std::filesystem::path& file);

	template<typename T>
	void readType(T* data, const size_t& customSize = 0)
	{
		const auto size(customSize == 0 ? sizeof(T) : customSize);
		std::memcpy(reinterpret_cast<char*>(data), m_readData, size);
		m_readData += size;
		m_readSize += size;
	}

	[[nodiscard]] char* GetDataPtr() { return m_readDataVector.data(); }
	[[nodiscard]] const std::vector<char>& GetData() const { return m_readDataVector; }
	[[nodiscard]] const std::filesystem::path& GetFilePath() const { return m_filePath; }
	[[nodiscard]] size_t GetCurrentReadSize() const { return m_readSize; }
private:
	std::vector<char> m_readDataVector{};
	std::filesystem::path m_filePath;
	char* m_readData = nullptr;
	size_t m_readSize = 0;
};
