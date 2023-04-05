#include "Header/MathFunctions.h"
#include <cmath>
#include <string>

bool MathFunctions::isEqual_f(const float a, const float b)
{
    return std::fabsf(a - b) < std::numeric_limits<float>::epsilon();
}

float MathFunctions::clamp_f(const float value, const float min, const float max)
{
	return std::fminf(max, std::fmaxf(value, min));
}

double MathFunctions::round_d_places(const double value, const uint32_t places)
{
	std::string placesStr("1");
	for(uint32_t i(0u); i < places; ++i) { placesStr.append("0"); }
	const auto convertedPlace(static_cast<double>(std::stoi(placesStr)));
	return std::ceil(value * convertedPlace) / convertedPlace;
}

float MathFunctions::round_f_places(const float value, const uint32_t places)
{
	std::string placesStr("1");
	for(uint32_t i(0u); i < places; ++i) { placesStr.append("0"); }
	const auto convertedPlace(static_cast<float>(std::stoi(placesStr)));
	return std::ceilf(value * convertedPlace) / convertedPlace;
}