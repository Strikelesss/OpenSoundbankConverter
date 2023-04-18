#pragma once
#include <string>
#include <vector>

namespace Logger
{
	inline std::vector<std::string> m_logMessages{};

    void LogToPlatform(const std::string& msg);
    
	template<typename... Args>
	void LogMessage(const std::string_view& msg, Args&&... args)
	{
		std::string stringBuffer;
		stringBuffer.resize(static_cast<size_t>(std::snprintf(nullptr, 0, msg.data(), std::forward<Args>(args)...)) + 1);
		const auto newSize(std::snprintf(stringBuffer.data(), stringBuffer.size(), msg.data(), std::forward<Args>(args)...));

		// Remove the null-terminating character
		stringBuffer.resize(newSize);

	    LogToPlatform(stringBuffer);
		m_logMessages.emplace_back(std::move(stringBuffer));
	}

	inline const std::vector<std::string>& GetLogMessages() { return m_logMessages; }
}