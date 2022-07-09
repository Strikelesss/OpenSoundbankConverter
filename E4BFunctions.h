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
    int32_t a(0u);
    is >> a;
    extType = static_cast<EExtractionType>(a);
    return is;
}

namespace E4BFunctions
{
	bool replaceStringOccurrance(std::string& str, const std::string& from, const std::string& to);
	int VerifyChunkName(const Chunk* chunk, const char* name);
	int GetSampleChannels(const E3Sample* sample);
	void ExtractSampleData(E3Sample* sample, unsigned int len);
	[[nodiscard]] bool ProcessE4BFile(std::vector<char>& bankData, EExtractionType extType, E4Result& outResult, const std::filesystem::path& wavFileFormat = "");
}