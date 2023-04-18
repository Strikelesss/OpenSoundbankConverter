#pragma once
#include "Header/MathFunctions.h"
#include <assert.h>
#include <filesystem>

enum struct EReaderFlags final
{
    NONE = 0u,
    BYTESWAP_RESULT = 1u
};

struct BinaryReader final
{
	BinaryReader() = default;
	bool readFile(const std::filesystem::path& file);

	template<typename T>
    void readType(T* data, const size_t& size = sizeof(T), const EReaderFlags flags = EReaderFlags::NONE)
    {
	    static_assert(std::is_fundamental_v<T>);
	    
        const bool valid(m_readData != nullptr && size != 0);
        assert(valid);
        if (valid)
        {
            std::memcpy(reinterpret_cast<char*>(data), m_readData, size);
            m_readData += size;
            m_readLocation += size;

            if (flags != EReaderFlags::NONE)
            {
                if constexpr (std::is_same_v<T, uint16_t>)
                {
                    *data = _byteswap_ushort(*data);
                }
                else if constexpr (std::is_same_v<T, uint32_t>)
                {
                    *data = MathFunctions::byteswapUINT32(*data);
                }
                else
                {
                    // Invalid type
                    assert(false);
                }
            }
        }
    }

	template<typename T>
	void readTypeAtLocation(T* data, const size_t& location, const size_t& size = sizeof(T), const EReaderFlags flags = EReaderFlags::NONE)
	{
        static_assert(std::is_fundamental_v<T>);
	    
        const bool valid(size != 0 && location + size <= m_readDataVector.size());
        assert(valid);
        if (valid)
        {
            std::memcpy(reinterpret_cast<char*>(data), &m_readDataVector[location], size);

            if (flags != EReaderFlags::NONE)
            {
                if constexpr (std::is_same_v<T, uint16_t>)
                {
                    *data = _byteswap_ushort(*data);
                }
                else if constexpr (std::is_same_v<T, uint32_t>)
                {
                    *data = MathFunctions::byteswapUINT32(*data);
                }
                else
                {
                    // Invalid type
                    assert(false);
                }
            }
        }
	}

	void skipBytes(const size_t numBytes)
	{
	    const bool valid(!m_readDataVector.empty() && numBytes <= m_readDataVector.size()
            && numBytes + m_readLocation <= m_readDataVector.size());
	    
	    assert(valid);
		if(valid)
		{
			m_readData += numBytes;
			m_readLocation += numBytes;
		}
	}

	[[nodiscard]] const std::vector<char>& GetData() const { return m_readDataVector; }
    [[nodiscard]] size_t GetReadLocation() const { return m_readLocation; }
    
private:
	std::vector<char> m_readDataVector{};
	std::filesystem::path m_filePath;
	char* m_readData = nullptr;
	size_t m_readLocation = 0;
};

struct ReadLocationHandle final
{
    explicit ReadLocationHandle(BinaryReader& reader, const size_t startingLocation) : m_reader(&reader), m_dataOffset(startingLocation) {}
    
    template<typename T>
    void readType(T* data, const size_t& size = sizeof(T), const EReaderFlags flags = EReaderFlags::NONE)
    {
        m_reader->readTypeAtLocation(data, m_dataOffset, size, flags);
        m_dataOffset += size;
    }
    
    BinaryReader* m_reader = nullptr;
    size_t m_dataOffset = 0;
};