#include "math.h"
#include <cmath>

namespace killme
{
	const float PI = 3.14159265358979323846264338327950288f;
	const float PI_2 = PI / 2;
	const float PI_4 = PI / 4;

	float radian(float deg)
	{
		return deg * PI / 180;
	}

	float degree(float rad)
	{
		return rad * 180 / PI;
	}

	bool equalf(float a, float b)
	{
		static const auto EPSILON = 0.0001f;
		return std::abs(a - b) < EPSILON;
	}
}