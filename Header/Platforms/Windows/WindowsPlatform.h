#pragma once
#include <filesystem>

namespace WindowsPlatform
{
    [[nodiscard]] std::filesystem::path GetSaveFolder();
    [[nodiscard]] std::filesystem::path GetSavePath(const std::string_view& filename);
};