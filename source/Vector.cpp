#include "Vector.h"

eVec2 vec2_zero(0.0f, 0.0f);
eVec2 vec2_oneZero(1.0f, 0.0f);
eVec2 vec2_zeroOne(0.0f, 1.0f);
eVec2 vec2_one(1.0f, 1.0f);

eVec3 vec3_zero(0.0f, 0.0f, 0.0f);
eVec3 vec3_oneZero(1.0f, 0.0f, 0.0f);
eVec3 vec3_one(1.0f, 1.0f, 1.0f);

eQuat rotateCounterClockwiseZ(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));
eQuat rotateClockwiseZ(0.0f, 0.0f, SDL_sinf(DEG2RAD(-ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(-ROTATION_INCREMENT) / 2.0f));

//******************
// eVec2::Rotate
// moves the point to the local space of origin, 
// rotates it, then moves it back to world space
//******************
void eVec2::Rotate(float degrees, const eVec2 & origin) {
	const float cosAngle = SDL_cosf(DEG2RAD(degrees));
	const float sinAngle = SDL_sinf(DEG2RAD(degrees));
	const eVec2 R[2] = {	eVec2(cosAngle, sinAngle),
							eVec2(-sinAngle, cosAngle) };

	// move to the local space
	eVec2 localPoint = *this - origin;
	*this = origin + eVec2(R[0].x * localPoint.x + R[1].x * localPoint.y,
		R[0].y * localPoint.x + R[1].y * localPoint.y);
}