#pragma once
#include <filesystem>

struct BankWriteOptions final
{
    BankWriteOptions() = default;

    // General options
    
    bool m_useConverterSpecificData = true;

    // Saving
    
    std::filesystem::path m_saveFolder;
};