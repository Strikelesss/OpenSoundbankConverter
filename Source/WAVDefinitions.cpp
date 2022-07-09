#include "Header/WAVDefinitions.h"
#include <mmreg.h>

HRESULT WAVDefinitions::FindChunk(const HANDLE hFile, const DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr(0);
	if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) // If unable to set file pointer to file handle
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwChunkType(0);
	DWORD dwChunkDataSize(0);
	DWORD dwRIFFDataSize(0);
	DWORD dwFileType(0);
	DWORD dwOffset(0);

	while (!FAILED(hr)) // while no errors
	{
		DWORD dwRead(0);
		if (ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, nullptr) == 0)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			return hr;
		}

		if (ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, nullptr) == 0)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			return hr;
		}

		if (dwChunkType == fourccRIFF)
		{
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, nullptr) == 0)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				return hr;
			}
		}
		else
		{
			if (SetFilePointer(hFile, static_cast<LONG>(dwChunkDataSize), nullptr, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				return hr;
			}
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		// we were checking an unset variable before with 0, is this even right?
		if (0 >= dwRIFFDataSize) return S_FALSE;
	}

	return S_OK;
}

HRESULT WAVDefinitions::ReadChunkData(const HANDLE hFile, void* buffer, const DWORD bufferSize, const LONG bufferOffset)
{
	HRESULT hr(0);
	if (SetFilePointer(hFile, bufferOffset, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	DWORD dwRead(0);
	if (ReadFile(hFile, buffer, bufferSize, &dwRead, nullptr) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	return hr;
}

bool WAVDefinitions::LoadWavExtensibleFile(const std::filesystem::path& filePath, std::vector<unsigned char>& outData)
{
	const auto hFile(CreateFile(filePath.c_str(), GENERIC_READ,
		FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr));

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return false;
	}

	// Locate RIFF Chunk in audio file

	DWORD dwChunkSize(0);
	DWORD dwChunkPosition(0);

	// check the file type, should be fourccWAVE or 'XWMA'
	if (FAILED(FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition)))
	{
		return false;
	}

	DWORD filetype(0);
	if (FAILED(ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition)))
	{
		return false;
	}

	if (filetype != fourccWAVE)
	{
		return false;
	}

	// Locate 'fmt' chunk, copy contents into format wfx
	if (FAILED(FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition)))
	{
		return false;
	}

	WAVEFORMATEXTENSIBLE wfx{};
	if (FAILED(ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition)))
	{
		return false;
	}

	// Locate 'data' chunk, read contents into buffer
	// fill out the audio data buffer with the contents of the fourccDATA chunk
	if (FAILED(FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition)))
	{
		return false;
	}

	outData.resize(dwChunkSize);
	if (FAILED(ReadChunkData(hFile, outData.data(), dwChunkSize, static_cast<LONG>(dwChunkPosition))))
	{
		return false;
	}

	return true;
}