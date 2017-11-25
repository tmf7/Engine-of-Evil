#ifndef EVIL_MATH_H
#define EVIL_MATH_H

#include <math.h>

#define RAD2DEG(radians) ( radians * (180.0f/((float)M_PI)) )
#define DEG2RAD(degrees) ( degrees * ((float)(M_PI)/180.0f) )

enum class COMPARE_ENUM {
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	EQUAL,
	NOT_EQUAL
};


//***********************************
//			eMath
//	 general math utilities
//***********************************
class eMath {
public:
	static float		NearestFloat(float x);
	static int			NearestInt(float x);
	static void			IsometricToCartesian(float & x, float & y);
	static void			IsometricToCartesian(int & x, int & y);
	static void			CartesianToIsometric(float & x, float & y);
	static void			CartesianToIsometric(int & x, int & y);
	static float		GetAngle(float x, float y);

						template<class type>
	static bool			CompareUtility(const type & lhs, COMPARE_ENUM compare, const type & rhs);
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
// eMath::IsometricToCartesian
// rotates input coordinates 45 degrees counter-clockwise
// DEBUG: assumes the input coordinates are isometric
//************
inline void eMath::IsometricToCartesian(int & x, int & y) {
	float isoX = (float)x;
	float isoY = (float)y;
	x = NearestInt((2.0f * isoY + isoX) * 0.5f);
	y = NearestInt((2.0f * isoY - isoX) * 0.5f);
}

//************
// eMath::CartesianToIsometric
// rotates input coordinates 45 degrees clockwise 
// DEBUG: assumes the input coordinates are cartesian
//************
inline void eMath::CartesianToIsometric(float & x, float & y) {
	float cartX = x;
	float cartY = y;
	x = cartX - cartY;
	y = (cartX + cartY) * 0.5f;
}

//************
// eMath::CartesianToIsometric
// rotates input coordinates 45 degrees clockwise 
// DEBUG: assumes the input coordinates are cartesian
//************
inline void eMath::CartesianToIsometric(int & x, int & y) {
	int cartX = x;
	int cartY = y;
	x = cartX - cartY;
	y = NearestInt((float)(cartX + cartY) * 0.5f);
}

//*****************
// eMath::GetAngle
// returns the angle in degrees from the given components
// DEBUG: assumes components are part of a normalized vector
//*****************
inline float eMath::GetAngle(float x, float y) {
	float angle = 0.0f;
	if (x == 0.0f && y > 0.0f) {
		return 90.0f;
	} else if (x == 0.0f && y < 0.0f) {
		return 270.0f;
	} else {
		float tan = y / x;
		angle = RAD2DEG(atanf(tan));
		if (x < 0)
			angle += 180.0f;
	}
	return angle;
}

//*****************
// eMath::CompareUtility
// allows for runtime comparision operator changes
//*****************
template<class type>
bool eMath::CompareUtility(const type & lhs, COMPARE_ENUM compare, const type & rhs) {
	switch (compare) {
		case COMPARE_ENUM::LESS:			return lhs <  rhs;
		case COMPARE_ENUM::LESS_EQUAL:		return lhs <= rhs;
		case COMPARE_ENUM::GREATER:			return lhs >  rhs;
		case COMPARE_ENUM::GREATER_EQUAL:	return lhs >= rhs;
		case COMPARE_ENUM::EQUAL:			return lhs == rhs;
		case COMPARE_ENUM::NOT_EQUAL:		return lhs != rhs;
		default:							return false;		// invalid comparator
	}	
}

#endif /* EVIL_MATH_H */

