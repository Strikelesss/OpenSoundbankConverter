#pragma once
#include <filesystem>

struct BinaryReader;
struct E4Result;
struct E4Sample;

enum struct EExtractionType
{
	PRINT_INFO,
	WAV_REPLACEMENT
};

inline std::istream& operator>>(std::istream& is, EExtractionType& extType)
{
    int32_t a(0);
    is >> a;
    extType = static_cast<EExtractionType>(a);
    return is;
}

namespace E4BFunctions
{
	[[nodiscard]] uint32_t GetSampleChannels(const E4Sample& sample);
	[[nodiscard]] bool ProcessE4BFile(BinaryReader& reader, EExtractionType extType, E4Result& outResult);
}