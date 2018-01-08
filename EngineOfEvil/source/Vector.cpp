/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#include "Vector.h"

using namespace evil;

eVec2 vec2_zero(0.0f, 0.0f);
eVec2 vec2_oneZero(1.0f, 0.0f);
eVec2 vec2_zeroOne(0.0f, 1.0f);
eVec2 vec2_one(1.0f, 1.0f);
eVec2 vec2_epsilon(FLT_EPSILON, FLT_EPSILON);

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