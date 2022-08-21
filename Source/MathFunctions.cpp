#include "Header/MathFunctions.h"
#include <cmath>

float MathFunctions::clamp_f(const float x, const float upper, const float lower)
{
	return std::fminf(upper, std::fmaxf(x, lower));
}