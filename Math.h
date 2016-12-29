#ifndef EVIL_MATH_H
#define EVIL_MATH_H

#include <math.h>

//***********************************
//			eMath
// general math functionality
//***********************************
class eMath {
public:
	static float NearestFloat(float x);
	static int NearestInt(float x);
	static void IsometricToCartesian(float & x, float & y);
	static void CartesianToIsometric(float & x, float & y);
};

//************
// eMath::NearestFloat
//************
inline float eMath::NearestFloat(float x) {
	return floorf(x + 0.5f);
}

//************
// eMath::NearestInt
//************
inline int eMath::NearestInt(float x) {
	return (int)(x + 0.5f);
}

//************
// eMath::IsometricToCartesian
// rotates input coordinates 45 degrees counter-clockwise
// DEBUG: assumes the input coordinates are isometric
//************
inline void eMath::IsometricToCartesian(float & x, float & y) {
	float isoX = x;
	float isoY = y;
	x = (2.0f * isoY + isoX) * 0.5f;
	y = (2.0f * isoY - isoX) * 0.5f;
}

//************
// eMath::CartesianToIsometric
// // rotates input coordinates 45 degrees clockwise 
// DEBUG: assumes the input coordinates are cartesian
//************
inline void eMath::CartesianToIsometric(float & x, float & y) {
	float cartX = x;
	float cartY = y;
	x = cartX - cartY;
	y = (cartX + cartY) * 0.5f;
}

#endif /* EVIL_MATH_H */

