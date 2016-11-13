#ifndef EVIL_MATH_H
#define EVIL_MATH_H

#include <math.h>

class eMath {
public:
	static float NearestFloat(float x);
	static int NearestInt(float x);
};

inline float eMath::NearestFloat(float x) {
	return floorf(x + 0.5f);
}

inline int eMath::NearestInt(float x) {
	return (int)(x + 0.5f);
}

#endif /* EVIL_MATH_H */

