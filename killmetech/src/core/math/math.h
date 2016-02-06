#ifndef _KILLME_MATH_H_
#define _KILLME_MATH_H_

namespace killme
{
	/** Math constant definitions */
	extern const float PI;		/// 3.141592...
	extern const float PI_2;	/// PI/2
	extern const float PI_4;	/// PI/4

	/** Convert a degree angle to radian */
	float radian(float deg);

	/** Convert a radian angle to degree */
	float degree(float rad);

	/** Equivalent test for the floating point */
	bool equalf(float a, float b);
}

#endif