#ifndef EVIL_VECTOR_H
#define EVIL_VECTOR_H

#include "SDL.h"			// for SDL_sqrtf
#include "Math.h"

#define ROTATION_INCREMENT 1.0f

// handles vectors, quaternions, and their operations

class eVec3;

//******************
// eVec2
//******************
class eVec2 {
public:

	float		x;
	float		y;

				eVec2();
	explicit	eVec2(const float x, const float y);

	void		Set(const float x, const float y);
	void		Zero();
	float		Normalize();
	eVec2		Normalized();
	float		Length() const;
	float		LengthSquared() const;
	void		Rotate(float degrees, const eVec2 & origin);

	float		operator[](const int index) const;
	float &		operator[](const int index);
	eVec2		operator-() const;
	float		operator*(const eVec2 &a) const;
	eVec2		operator*(const float a) const;
	eVec2		operator/(const float a) const;
	eVec2		operator+(const eVec2 &a) const;
	eVec2		operator-(const eVec2 &a) const;
	eVec2 &		operator+=(const eVec2 &a);
	eVec2 &		operator-=(const eVec2 &a);
	eVec2 &		operator/=(const float a);
	eVec2 &		operator*=(const float a);

	bool		Compare(const eVec2 &a) const;
	bool		Compare(const eVec2 &a, const float epsilon) const;
	bool		operator==(const eVec2 &a) const;
	bool		operator!=(const eVec2 &a) const;

	void		SnapInt();
};

extern eVec2 vec2_zero;
extern eVec2 vec2_oneZero;
extern eVec2 vec2_zeroOne;
extern eVec2 vec2_one;

//******************
// eVec2::eVec2
//******************
inline eVec2::eVec2()
	: x(0.0f),
	  y(0.0f) {
}

//******************
// eVec2::eVec2
//******************
inline eVec2::eVec2(const float x, const float y) 
	: x(x),
	  y(y) {
}

//******************
// eVec2::Set
// modifies the current x and y values
//******************
inline void eVec2::Set(const float x, const float y) {
	this->x = x;
	this->y = y;
}

//******************
// eVec2::Zero
// sets x and y to zero
//******************
inline void eVec2::Zero() {
	x = 0.0f;
	y = 0.0f;
}

//******************
// eVec2::Compare
// returns true for exact x,y match
//******************
inline bool eVec2::Compare(const eVec2 &a) const {
	return ((x == a.x) && (y == a.y));
}

//******************
// eVec2::Compare
// returns true for x,y match within range of epsilon
//******************
inline bool eVec2::Compare(const eVec2 &a, const float epsilon) const {
	if ( SDL_fabs(x - a.x) > epsilon)
		return false;

	if (SDL_fabs(y - a.y) > epsilon)
		return false;

	return true;
}

//******************
// eVec2::operator[]
// returns x for [0] and y for [1]
// undefined behavior for index out of bounds
//******************
inline float eVec2::operator[](const int index) const {
	return (&x)[index];
}

//******************
// eVec2::operator[]
// returns x for [0] and y for [1]
// undefined behavior for index out of bounds
//******************
inline float & eVec2::operator[](const int index) {
	return (&x)[index];
}

//******************
// eVec2::operator==
// returns the result of Compare without epsilon
//******************
inline bool eVec2::operator==(const eVec2 &a) const {
	return Compare(a);
}

//******************
// eVec2::operator!=
// returns the negated result of Compare without epsilon
//******************
inline bool eVec2::operator!=(const eVec2 &a) const {
	return !Compare(a);
}

//******************
// eVec2::Length
// returns vector length
//******************
inline float eVec2::Length() const {
	return SDL_sqrtf(x * x + y * y);
}

//******************
// eVec2::LengthSquared
// returns vector length squared
//******************
inline float eVec2::LengthSquared() const {
	return (x * x + y * y);
}

//******************
// eVec2::Normalize
// scales internal x,y to unit length vector
// returns normalized length of 0 or 1
//******************
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

//******************
// eVec2::Normalized
// scales a copy of x,y to unit length vector
//******************
inline eVec2 eVec2::Normalized() {
	float sqrLength;
	float invLength;

	sqrLength = x * x + y * y;
	sqrLength = SDL_sqrtf(sqrLength);

	if (sqrLength == 0)
		invLength = 0;
	else
		invLength = 1.0f / sqrLength;

	return eVec2(x * invLength, y * invLength);
}

//******************
// eVec2::operator-
// returns eVec2 with negated x and y
//******************
inline eVec2 eVec2::operator-() const {
	return eVec2(-x, -y);
}

//******************
// eVec2::operator-
// returns an eVec2 using the difference between two vectors' x and y
//******************
inline eVec2 eVec2::operator-(const eVec2 &a) const {
	return eVec2(x - a.x, y - a.y);
}

//******************
// eVec2::operator*
// returns the dot product of two eVec2s
//******************
inline float eVec2::operator*(const eVec2 &a) const {
	return x * a.x + y * a.y;
}

//******************
// eVec2::operator*
// returns an eVec2 with x and y scaled by a
//******************
inline eVec2 eVec2::operator*(const float a) const {
	return eVec2(x * a, y * a);
}

//******************
// eVec2::operator/
// returns an eVec2 with de-scaled x and y by a
// allows divide by zero errors to occur
//******************
inline eVec2 eVec2::operator/(const float a) const {
	float inva = 1.0f / a;
	return eVec2(x * inva, y * inva);
}

//******************
// eVec2::operator+
// returns an eVec2 using the sum of two eVec2's x and y
//******************
inline eVec2 eVec2::operator+(const eVec2 &a) const {
	return eVec2(x + a.x, y + a.y);
}

//******************
// eVec2::operator+=
// returns this with x and y added to an eVec2's x and y
//******************
inline eVec2 & eVec2::operator+=(const eVec2 &a) {
	x += a.x;
	y += a.y;
	return *this;
}

//******************
// eVec2::operator/=
// returns this with x and y de-scaled by a
// allows divide by zero error to occur
//******************
inline eVec2 & eVec2::operator/=(const float a) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	return *this;
}

//******************
// eVec2::operator-=
// returns this with x and y subtracted by another eVec2's x and y
//******************
inline eVec2 & eVec2::operator-=(const eVec2 &a) {
	x -= a.x;
	y -= a.y;
	return *this;
}

//******************
// eVec2::operator*=
// returns this with x and y scaled by a
//******************
inline eVec2 & eVec2::operator*=(const float a) {
	x *= a;
	y *= a;
	return *this;
}

//******************
// eVec2::SnapInt
// moves x and y to the nearest integer value
//******************
inline void eVec2::SnapInt() {
	x = floorf(x + 0.5f);
	y = floorf(y + 0.5f);
}

//******************
// eVec3
//******************
class eVec3 {
public:

	float		x;
	float		y;
	float		z;

				eVec3();
	explicit	eVec3(const float x, const float y, const float z);
				eVec3(const eVec2 & point);

	void		Set(const float x, const float y, const float z);
	void		Zero();
	float		Normalize();
	float		Length() const;
	float		LengthSquared() const;

	float		operator[](const int index) const;
	float &		operator[](const int index);
	eVec3		operator-() const;
	float		operator*(const eVec3 &a) const;
	eVec3		operator*(const float a) const;
	eVec3		operator/(const float a) const;
	eVec3		operator+(const eVec3 &a) const;
	eVec3		operator-(const eVec3 &a) const;
	eVec3 &		operator+=(const eVec3 &a);
	eVec3 &		operator-=(const eVec3 &a);
	eVec3 &		operator/=(const float a);
	eVec3 &		operator*=(const float a);	

	bool		Compare(const eVec3 &a) const;
	bool		Compare(const eVec3 &a, const float epsilon) const;
	bool		operator==(const eVec3 &a) const;
	bool		operator!=(const eVec3 &a) const;

	eVec3		Cross(const eVec3 &a) const;
	eVec3 &		Cross(const eVec3 &a, const eVec3 &b);

	void		SnapInt();
};

extern eVec3 vec3_zero;
extern eVec3 vec3_oneZero;
extern eVec3 vec3_one;

//******************
// eVec3::eVec3
//******************
inline eVec3::eVec3()
	: x(0.0f),
	  y(0.0f),
	  z(0.0f) {
}

//******************
// eVec3::eVec3
//******************
inline eVec3::eVec3(const float x, const float y, const float z)
	: x(x),
	  y(y),
	  z(z) {
}

//******************
// eVec3::eVec3
//******************
inline eVec3::eVec3(const eVec2 & point)
	: x(point.x),
	  y(point.y),
	  z(0.0f) {
}

//******************
// eVec3::Set
// modifies the current x, y, and z values
//******************
inline void eVec3::Set(const float x, const float y, const float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

//******************
// eVec3::Zero
// sets x, y, and z to zero
//******************
inline void eVec3::Zero() {
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

//******************
// eVec3::Compare
// returns true for exact x,y,z match
//******************
inline bool eVec3::Compare(const eVec3 &a) const {
	return ((x == a.x) && (y == a.y) && (z == a.z));
}

//******************
// eVec3::Compare
// returns true for x,y,z match within range of epsilon
//******************
inline bool eVec3::Compare(const eVec3 &a, const float epsilon) const {

	if (SDL_fabs(x - a.x) > epsilon)
		return false;

	if (SDL_fabs(y - a.y) > epsilon)
		return false;

	if (SDL_fabs(z - a.z) > epsilon)
		return false;

	return true;
}

//******************
// eVec3::operator[]
// returns x for [0], y for [1], and z for [2]
// undefined behavior for index out of bounds
//******************
inline float eVec3::operator[](const int index) const {
	return (&x)[index];
}

//******************
// eVec3::operator[]
// returns x for [0], y for [1], and z for [2]
// undefined behavior for index out of bounds
//******************
inline float & eVec3::operator[](const int index) {
	return (&x)[index];
}

//******************
// eVec3::operator==
// returns the result of Compare without epsilon
//******************
inline bool eVec3::operator==(const eVec3 &a) const {
	return Compare(a);
}

//******************
// eVec3::operator!=
// returns the negated result of Compare without epsilon
//******************
inline bool eVec3::operator!=(const eVec3 &a) const {
	return !Compare(a);
}

//******************
// eVec3::Length
// returns vector length
//******************
inline float eVec3::Length() const {
	return SDL_sqrtf(x * x + y * y + z * z);
}

//******************
// eVec3::LengthSquared
// returns vector length squared
//******************
inline float eVec3::LengthSquared() const {
	return (x * x + y * y + z * z);
}

//******************
// eVec3::Normalize
// scales internal x,y, and z to unit length vector
// returns normalized length of 0 or 1
//******************
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

//******************
// eVec3::operator-
// returns eVec3 with negated x, y, and z
//******************
inline eVec3 eVec3::operator-() const {
	return eVec3(-x, -y, -z);
}

//******************
// eVec3::operator-
// returns an eVec3 using the difference between two vectors' x, y, and z
//******************
inline eVec3 eVec3::operator-(const eVec3 &a) const {
	return eVec3(x - a.x, y - a.y, z - a.z);
}

//******************
// eVec3::operator*
// returns the dot product of two eVec3s
//******************
inline float eVec3::operator*(const eVec3 &a) const {
	return x * a.x + y * a.y + z * a.z;
}

//******************
// eVec3::operator*
// returns an eVec3 with x, y, and z scaled by a
//******************
inline eVec3 eVec3::operator*(const float a) const {
	return eVec3(x * a, y * a, z * a);
}

//******************
// eVec3::operator/
// returns an eVec3 with de-scaled x, y, and z by a
// allows divide by zero errors to occur
//******************
inline eVec3 eVec3::operator/(const float a) const {
	float inva = 1.0f / a;
	return eVec3(x * inva, y * inva, z * inva);
}

//******************
// eVec3::operator+
// returns an eVec3 using the sum of two eVec3's x, y, and z
//******************
inline eVec3 eVec3::operator+(const eVec3 &a) const {
	return eVec3(x + a.x, y + a.y, z + a.z);
}

//******************
// eVec3::operator+=
// returns this with x, y, and z added to an eVec3's x, y, and z
//******************
inline eVec3 & eVec3::operator+=(const eVec3 &a) {
	x += a.x;
	y += a.y;
	z += a.z;
	return *this;
}

//******************
// eVec3::operator/=
// returns this with x, y, and z de-scaled by a
// allows divide by zero error to occur
//******************
inline eVec3 & eVec3::operator/=(const float a) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;
	return *this;
}

//******************
// eVec3::operator-=
// returns this with x, y, and z subtracted by another eVec3's x, y, and z
//******************
inline eVec3 & eVec3::operator-=(const eVec3 &a) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	return *this;
}

//******************
// eVec3::operator*=
// returns this with x, y, and z scaled by a
//******************
inline eVec3 & eVec3::operator*=(const float a) {
	x *= a;
	y *= a;
	z *= a;
	return *this;
}

//******************
// eVec3::Cross
// returns the an eVec3 that is the cross product of *this with another eVec3
//******************
inline eVec3 eVec3::Cross(const eVec3 &a) const {
	return eVec3( y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
}

//******************
// eVec3::Cross
// sets *this to the cross product of two eVec3s
//******************
inline eVec3 & eVec3::Cross(const eVec3 &a, const eVec3 &b) {
	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;
	return *this;
}

//******************
// eVec3::SnapInt
// moves x, y, and z to the nearest integer value
//******************
inline void eVec3::SnapInt() {
	x = floorf(x + 0.5f);
	y = floorf(y + 0.5f);
	z = floorf(z + 0.5f);
}

//******************
// eQuat
// --only deals with unit length quaternions--
//******************
class eQuat {
public:

	float			x;
	float			y;
	float			z;
	float			w;

					eQuat();
	explicit		eQuat(const float x, const float y, const float z, const float w);
	explicit		eQuat(const eVec3 vector, const float scalar);

	eQuat			Inverse() const;
	void			Set(const float x, const float y, const float z, const float w);

	float			operator[](const int index) const;
	float &			operator[](const int index);
	eQuat			operator*(const eQuat &a) const;
	eVec3			operator*(const eVec3 &a) const;
	eVec2			operator*(const eVec2 &a) const;
	eQuat &			operator*=(const eQuat &a);
};

extern eQuat rotateCounterClockwiseZ;	// rotate ROTATION_INCREMENT counter-clockwise about z-axis
extern eQuat rotateClockwiseZ;			// rotate ROTATION_INCREMENT clockwise about z-axis

//******************
// eQuat::eQuat
//******************
inline eQuat::eQuat()
	: x(0.0f),
	  y(0.0f),
	  z(0.0f),
	  w(0.0f) {
}

//******************
// eQuat::eQuat
//******************
inline eQuat::eQuat(const float x, const float y, const float z, const float w)
	: x(x),
	  y(y),
	  z(z),
	  w(w) {
}

//******************
// eQuat::eQuat
//******************
inline eQuat::eQuat(const eVec3 vector, const float scalar)
	: x(vector.x),
	  y(vector.y),
	  z(vector.z),
	  w(scalar) {
}

//******************
// eQuat::operator[]
// returns x for [0], y for [1], z for [2], and w for [3]
// undefined behavior for index out of bounds
//******************
inline float eQuat::operator[](const int index) const {
	return (&x)[index];
}

//******************
// eQuat::operator[]
// returns x for [0], y for [1], z for [2], and w for [3]
// undefined behavior for index out of bounds
//******************
inline float & eQuat::operator[](const int index) {
	return (&x)[index];
}

//******************
// eQuat::operator*
// returns an eQuat set to the Grassman product of *this with another eQuat
// this*a = q*p = [ (qs*pv + ps*qv + qv X pv) (qs*ps - qv.pv) ]
//******************
inline eQuat eQuat::operator*(const eQuat &a) const {
	return eQuat(w*a.x + x*a.w + y*a.z - z*a.y,
				 w*a.y + y*a.w + z*a.x - x*a.z,
				 w*a.z + z*a.w + x*a.y - y*a.x,
				 w*a.w - x*a.x - y*a.y - z*a.z);
}

//******************
// eQuat::operator*=
// returns *this set to the Grassman product of *this with another eQuat
// this*a = q*p = [ (qs*pv + ps*qv + qv X pv) (qs*ps - qv.pv) ]
//******************
inline eQuat & eQuat::operator*=(const eQuat &a) {
	*this = *this * a;
	return *this;
}

//******************
// eQuat::operator*
// expanded and factored version of:
// (*this) * eQuat( a.x, a.y, a.z, 0.0f ) * (*this)^-1
// returns an eVec3 by rotating the given eVec3 
// counter-clockwise using *this eQuat.
// assumes a right-handed coordinate system
// --only deals with unit length quaternions--
//******************
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

//******************
// eQuat::operator*
// expanded and factored version of:
// (*this) * eQuat( a.x, a.y, 0.0f, 0.0f ) * (*this)^-1
// returns an eVec2 by rotating the given eVec2 
// counter-clockwise using *this eQuat.
// assumes a right-handed coordinate system
// --only deals with unit length quaternions--
//******************
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

//******************
// eQuat::Inverse
// returns an eQuat that is the inverse of *this
// unit length quaternion have inverse == conjugate
//******************
inline eQuat eQuat::Inverse() const {
	return eQuat(-x, -y, -z, w);
}

//******************
// eQuat::Set
// sets this x,y,z, and w to the given values
//******************
inline void eQuat::Set(const float x, const float y, const float z, const float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

#endif /* EVIL_VECTOR_H */

