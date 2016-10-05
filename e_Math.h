#ifndef E_MATH_H
#define E_MATH_H

#include "SDL.h"			// for SDL_fabs

#define DEG2RAD(angle)	( angle*((float)(M_PI)/180.0f) )
#define ORIGIN_VEC2		eVec2( 1.0f, 0.0f )
#define ZERO_VEC2		eVec2( 0.0f, 0.0f )
#define ZERO_VEC3		eVec3( 0.0f, 0.0f, 0.0f )
#define ORIGIN_VEC3		eVec3( 1.0f, 0.0f, 0.0f )

// handles vectors, quaternions, and their operations

//******************
// eVec2
//******************
class eVec2
{

public:

	float x;
	float y;

				eVec2();
	explicit	eVec2(const float x, const float y);

	void		Set(const float x, const float y);
	void		Zero();
	float		Normalize();												// returns length (should be 1 or 0)
	float		Length() const;
	float		LengthSquared() const;											

	eVec2		operator-() const;
	float		operator*(const eVec2 &a) const;							// dot product
	eVec2		operator*(const float a) const;								// rvalue scale
	eVec2		operator/(const float a) const;
	eVec2		operator+(const eVec2 &a) const;
	eVec2		operator-(const eVec2 &a) const;
	eVec2 &		operator+=(const eVec2 &a);
	eVec2 &		operator-=(const eVec2 &a);
	eVec2 &		operator/=(const float a);									// lvalue scale
	eVec2 &		operator*=(const float a);									// lvalue scale

	bool		Compare(const eVec2 &a) const;								// exact compare, no epsilon
	bool		Compare(const eVec2 &a, const float epsilon) const;			// compare with epsilon
	bool		operator==(const eVec2 &a) const;							// exact compare, no epsilon
	bool		operator!=(const eVec2 &a) const;							// exact compare, no epsilon


};

inline eVec2::eVec2() {
}

inline eVec2::eVec2(const float x, const float y) {
	this->x = x;
	this->y = y;
}

inline void eVec2::Set(const float x, const float y) {
	this->x = x;
	this->y = y;
}

inline void eVec2::Zero() {
	x = 0.0f;
	y = 0.0f;
}

inline bool eVec2::Compare(const eVec2 &a) const {
	return ((x == a.x) && (y == a.y));
}

inline bool eVec2::Compare(const eVec2 &a, const float epsilon) const {
	if ( SDL_fabs(x - a.x) > epsilon) {
		return false;
	}

	if (SDL_fabs(y - a.y) > epsilon) {
		return false;
	}

	return true;
}

inline bool eVec2::operator==(const eVec2 &a) const {
	return Compare(a);
}

inline bool eVec2::operator!=(const eVec2 &a) const {
	return !Compare(a);
}


inline float eVec2::Length() const {
	return SDL_sqrtf(x * x + y * y);
}

inline float eVec2::LengthSquared() const {
	return (x * x + y * y);
}

inline float eVec2::Normalize() {
	float sqrLength, invLength;

	sqrLength = x * x + y * y;
	sqrLength = SDL_sqrtf(sqrLength);

	if (sqrLength == 0)
		invLength = 0;
	else
		invLength = 1.0f / sqrLength;

	x *= invLength;
	y *= invLength;
	return invLength * sqrLength;
}

inline eVec2 eVec2::operator-() const {
	return eVec2(-x, -y);
}

inline eVec2 eVec2::operator-(const eVec2 &a) const {
	return eVec2(x - a.x, y - a.y);
}

inline float eVec2::operator*(const eVec2 &a) const {
	return x * a.x + y * a.y;
}

inline eVec2 eVec2::operator*(const float a) const {
	return eVec2(x * a, y * a);
}

inline eVec2 eVec2::operator/(const float a) const {
	float inva = 1.0f / a;
	return eVec2(x * inva, y * inva);
}


inline eVec2 eVec2::operator+(const eVec2 &a) const {
	return eVec2(x + a.x, y + a.y);
}

inline eVec2 & eVec2::operator+=(const eVec2 &a) {
	x += a.x;
	y += a.y;

	return *this;
}

inline eVec2 & eVec2::operator/=(const float a) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;

	return *this;
}

inline eVec2 & eVec2::operator-=(const eVec2 &a) {
	x -= a.x;
	y -= a.y;

	return *this;
}

inline eVec2 & eVec2::operator*=(const float a) {
	x *= a;
	y *= a;

	return *this;
}

//******************
// eVec3
//******************
class eVec3
{
public:

	float x;
	float y;
	float z;

				eVec3();
	explicit	eVec3(const float x, const float y, const float z);

	void		Set(const float x, const float y, const float z);
	void		Zero();
	float		Normalize();												// returns length (should be 1 or 0)
	float		Length() const;
	float		LengthSquared() const;

	eVec3		operator-() const;
	float		operator*(const eVec3 &a) const;							// dot product
	eVec3		operator*(const float a) const;								// rvalue scale
	eVec3		operator/(const float a) const;
	eVec3		operator+(const eVec3 &a) const;
	eVec3		operator-(const eVec3 &a) const;
	eVec3 &		operator+=(const eVec3 &a);
	eVec3 &		operator-=(const eVec3 &a);
	eVec3 &		operator/=(const float a);									// lvalue scale
	eVec3 &		operator*=(const float a);									// lvalue scale

	bool		Compare(const eVec3 &a) const;								// exact compare, no epsilon
	bool		Compare(const eVec3 &a, const float epsilon) const;			// compare with epsilon
	bool		operator==(const eVec3 &a) const;							// exact compare, no epsilon
	bool		operator!=(const eVec3 &a) const;							// exact compare, no epsilon

	eVec3		Cross(const eVec3 &a) const;
	eVec3 &		Cross(const eVec3 &a, const eVec3 &b);
};

inline eVec3::eVec3() {
}

inline eVec3::eVec3(const float x, const float y, const float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

inline void eVec3::Set(const float x, const float y, const float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

inline void eVec3::Zero() {
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

inline bool eVec3::Compare(const eVec3 &a) const {
	return ((x == a.x) && (y == a.y) && (z == a.z));
}

inline bool eVec3::Compare(const eVec3 &a, const float epsilon) const {
	if (SDL_fabs(x - a.x) > epsilon) {
		return false;
	}

	if (SDL_fabs(y - a.y) > epsilon) {
		return false;
	}

	if (SDL_fabs(z - a.z) > epsilon) {
		return false;
	}

	return true;
}

inline bool eVec3::operator==(const eVec3 &a) const {
	return Compare(a);
}

inline bool eVec3::operator!=(const eVec3 &a) const {
	return !Compare(a);
}


inline float eVec3::Length() const {
	return SDL_sqrtf(x * x + y * y + z * z);
}

inline float eVec3::LengthSquared() const {
	return (x * x + y * y + z * z);
}

inline float eVec3::Normalize() {
	float sqrLength, invLength;

	sqrLength = x * x + y * y + z * z;
	sqrLength = SDL_sqrtf(sqrLength);

	if (sqrLength == 0)
		invLength = 0;
	else
		invLength = 1.0f / sqrLength;

	x *= invLength;
	y *= invLength;
	z *= invLength;
	return invLength * sqrLength;
}

inline eVec3 eVec3::operator-() const {
	return eVec3(-x, -y, -z);
}

inline eVec3 eVec3::operator-(const eVec3 &a) const {
	return eVec3(x - a.x, y - a.y, z - a.z);
}

inline float eVec3::operator*(const eVec3 &a) const {
	return x * a.x + y * a.y + z * a.z;
}

inline eVec3 eVec3::operator*(const float a) const {
	return eVec3(x * a, y * a, z * a);
}

inline eVec3 eVec3::operator/(const float a) const {
	float inva = 1.0f / a;
	return eVec3(x * inva, y * inva, z * inva);
}


inline eVec3 eVec3::operator+(const eVec3 &a) const {
	return eVec3(x + a.x, y + a.y, z + a.z);
}

inline eVec3 & eVec3::operator+=(const eVec3 &a) {
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

inline eVec3 & eVec3::operator/=(const float a) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;

	return *this;
}

inline eVec3 & eVec3::operator-=(const eVec3 &a) {
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

inline eVec3 & eVec3::operator*=(const float a) {
	x *= a;
	y *= a;
	z *= a;

	return *this;
}

inline eVec3 eVec3::Cross(const eVec3 &a) const {
	return eVec3( y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
}

inline eVec3 & eVec3::Cross(const eVec3 &a, const eVec3 &b) {

	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;
	return *this;
}

//******************
// eQuat
// --only deals with unit length quaternions--
//******************
class eQuat
{

public:

	float x;
	float y;
	float z;
	float w;

	eQuat();
	explicit eQuat(const float x, const float y, const float z, const float w);
	explicit eQuat(const eVec3 vector, const float scalar);

	eQuat			Inverse() const;
	void			Set(const float x, const float y, const float z, const float w);

	eQuat			operator*(const eQuat &a) const;
	eVec3			operator*(const eVec3 &a) const;
	eVec2			operator*(const eVec2 &a) const;
	eQuat &			operator*=(const eQuat &a);
};

inline eQuat::eQuat() {
}

inline eQuat::eQuat(const float x, const float y, const float z, const float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

inline eQuat::eQuat(const eVec3 vector, const float scalar) {
	x = vector.x;
	y = vector.y;
	z = vector.z;
	w = scalar;
}

// Grassman product
// this*a = q*p = [ (qs*pv + ps*qv + qv X pv) (qs*ps - qv.pv) ]
inline eQuat eQuat::operator*(const eQuat &a) const {
	return eQuat(w*a.x + x*a.w + y*a.z - z*a.y,
				 w*a.y + y*a.w + z*a.x - x*a.z,
				 w*a.z + z*a.w + x*a.y - y*a.x,
				 w*a.w - x*a.x - y*a.y - z*a.z);
}

inline eQuat & eQuat::operator*=(const eQuat &a) {
	*this = *this * a;
	return *this;
}

// expanded and factored version of:
// (*this) * eQuat( a.x, a.y, a.z, 0.0f ) * (*this)^-1
// implies a counter-clockwise vector rotation for a right-handed coordinate system
// as stated at the top: this class only deals with unit length quaternions
inline eVec3 eQuat::operator*(const eVec3 &a) const {
	float xxzz = x*x - z*z;
	float wwyy = w*w - y*y;

	float xw2 = x*w*2.0f;
	float xy2 = x*y*2.0f;
	float xz2 = x*z*2.0f;
	float yw2 = y*w*2.0f;
	float yz2 = y*z*2.0f;
	float zw2 = z*w*2.0f;

	return eVec3(
		(xxzz + wwyy)*a.x + (xy2 - zw2)*a.y + (xz2 + yw2)*a.z,
		(xy2 + zw2)*a.x + (y*y + w*w - x*x - z*z)*a.y + (yz2 - xw2)*a.z,
		(xz2 - yw2)*a.x + (yz2 + xw2)*a.y + (wwyy - xxzz)*a.z 
	);
}

// expanded and factored version of:
// (*this) * eQuat( a.x, a.y, 0.0f, 0.0f ) * (*this)^-1
// implies a counter-clockwise vector rotation for a right-handed coordinate system
// as stated at the top: this class only deals with unit length quaternions
inline eVec2 eQuat::operator*(const eVec2 &a) const {
	float xxzz = x*x - z*z;
	float wwyy = w*w - y*y;

	float xy2 = x*y*2.0f;
	float zw2 = z*w*2.0f;

	return eVec2(
		(xxzz + wwyy)*a.x + (xy2 - zw2)*a.y,
		(xy2 + zw2)*a.x + (y*y + w*w - x*x - z*z)*a.y
	);
}

// unit length quaternion => inverse == conjugate
inline eQuat eQuat::Inverse() const {
	return eQuat(-x, -y, -z, w);
}

inline void eQuat::Set(const float x, const float y, const float z, const float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

#endif /* E_MATH_H */

