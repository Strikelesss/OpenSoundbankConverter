#include "Header/Logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

void Logger::LogToPlatform(const std::string& msg)
{
#ifdef _WIN32
    OutputDebugStringA(msg.c_str());
    OutputDebugStringA("\n");
#else
#error Platform logger not implemented!
#endif
}