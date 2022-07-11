#pragma once
#include <string>

struct E4Result;

struct BankConverter final
{
	[[nodiscard]] bool ConvertE4BToSF2(const E4Result& e4b, const std::string& bankName) const;
};