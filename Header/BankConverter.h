#pragma once

struct BankWriteOptions;
struct Soundbank;

namespace BankConverter
{
	[[nodiscard]] bool CreateSF2(const Soundbank& bank, const BankWriteOptions& options);
	[[nodiscard]] bool CreateE4B(const Soundbank& bank, const BankWriteOptions& options);
};