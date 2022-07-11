#pragma once
#include <filesystem>
#include <vector>

struct E4Result;
struct Chunk;
struct E3Sample;

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
	int32_t VerifyChunkName(const Chunk* chunk, const char* name);
	uint32_t GetSampleChannels(const E3Sample* sample);
	[[nodiscard]] bool ProcessE4BFile(std::vector<char>& bankData, EExtractionType extType, E4Result& outResult);
}