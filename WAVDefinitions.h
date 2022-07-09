#pragma once
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace WAVDefinitions
{
	constexpr unsigned int fourcc(const char a, const char b, const char c, const char d)
	{
		return static_cast<unsigned int>(a) | (static_cast<unsigned int>(b) << 8) |
			(static_cast<unsigned int>(c) << 16) | (static_cast<unsigned int>(d) << 24);
	}

	constexpr auto fourccRIFF = fourcc('R', 'I', 'F', 'F');
	constexpr auto fourccFMT = fourcc('f', 'm', 't', ' ');
	constexpr auto fourccWAVE = fourcc('W', 'A', 'V', 'E');
	constexpr auto fourccDATA = fourcc('d', 'a', 't', 'a');

	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, LONG bufferOffset);
	bool LoadWavExtensibleFile(const std::filesystem::path& filePath, std::vector<unsigned char>& outData);
}