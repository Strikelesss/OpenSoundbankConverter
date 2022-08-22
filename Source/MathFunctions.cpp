#include "Header/MathFunctions.h"
#include <cmath>
#include <string>

float MathFunctions::clamp_f(const float x, const float min, const float max)
{
	return std::fminf(max, std::fmaxf(x, min));
}

double MathFunctions::round_d_places(const double x, const unsigned int places)
{
	std::string placesStr("1");
	for(uint32_t i(0u); i < places; ++i) { placesStr.append("0"); }
	const auto convertedPlace(static_cast<double>(std::stoi(placesStr)));
	return std::ceil(x * convertedPlace) / convertedPlace;
}

float MathFunctions::round_f_places(const float x, const unsigned int places)
{
	std::string placesStr("1");
	for(uint32_t i(0u); i < places; ++i) { placesStr.append("0"); }
	const auto convertedPlace(static_cast<float>(std::stoi(placesStr)));
	return std::ceilf(x * convertedPlace) / convertedPlace;
}