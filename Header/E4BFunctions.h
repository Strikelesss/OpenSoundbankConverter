#pragma once
#include <cstdint>

struct BinaryReader;
struct E4Result;
struct E4Sample;

namespace E4BFunctions
{
	[[nodiscard]] uint32_t GetSampleChannels(const E4Sample& sample);
	[[nodiscard]] bool ProcessE4BFile(BinaryReader& reader, E4Result& outResult);
	bool IsAccountingForCords(const E4Result& result);
}