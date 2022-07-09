#pragma once

struct Chunk
{
	std::array<char, E4BVariables::CHUNK_NAME_LEN> name{};
	unsigned int len = 0u;
	char data[];
};