#pragma once
#include <filesystem>

struct BankReadOptions;
struct Soundbank;

namespace SF2Reader
{
    [[nodiscard]] Soundbank ProcessFile(const std::filesystem::path& file, const BankReadOptions& options);
};