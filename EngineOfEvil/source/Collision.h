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
#ifndef EVIL_COLLISION_H
#define EVIL_COLLISION_H

#include "Definitions.h"
#include "Bounds3D.h"
#include "Bounds.h"
#include "Box.h"

class eGridCell;
class eCollisionModel;
class eMap;

// TODO: incorperate the contact point in Collision_t for all geometry 
// (eg: GJK closest points on convex hulls)
// Collision_t
typedef struct Collision_s {
	eVec2				normal;					// surface normal of collided object (default ctor: x = 0.0f, y = 0.0f)
	float				fraction = -1.0f;		// fraction along a movement to first contact
	eCollisionModel *	owner = nullptr;		// collided object
} Collision_t;


//************************************
//			eCollision
// utility class for collision tests
// TODO: move some of these tests to their
// their respective collider shapes (eg: AABBAABBTest)
// and use a visitor pattern to call them
//***********************************
class eCollision {
public:

	static eBounds			GetBroadPhaseBounds(const eBounds & bounds, const eVec2 & dir, const float length);
	static void				GetCollisionNormal(const eVec2 & point, const eBounds & bounds, eVec2 & resultNormal);
	static void				GetCollisionNormal(eBounds self, const eVec2 & dir, const float length, const eBounds & other, Collision_t & collision);
	static void				GetAreaCells(eMap * onMap, const eBox & area, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(eMap * onMap, const eBounds & area, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(eMap * onMap, const eBounds & bounds, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(eMap * onMap, const eVec2 & begin, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells);
	
	static bool				OBBOBBTest(const eBox & a, const eBox & b);

	static bool				AABBContainsPoint(const eBounds & bounds, const eVec2 & point);
	static bool				AABBContainsAABB(const eBounds & self, const eBounds & other);
	static bool				AABBContainsPolyLine(const eBounds & bounds, const std::vector<eVec2> & polyLine);	
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static bool				SegmentAABBTest(const eVec2 & begin, const eVec2 & end, const eBounds & bounds);
	static bool				MovingAABBAABBTest(const eBounds & self, const eVec2 & dir, const float length, const eBounds & other, float & resultFraction);
	static bool				RayAABBTest(const eVec2 & begin, const eVec2 & dir, const float length, const eBounds & bounds, float & resultFraction);

	static bool				RayCast(eMap * onMap, std::vector<Collision_t> & collisions, const eVec2 & begin, const eVec2 & dir, const float length = FLT_MAX, bool ignoreStartInCollision = true);
	static bool				BoxCast(eMap * onMap, std::vector<Collision_t> & collisions, const eBounds & bounds, const eVec2 & dir, const float length);

	static bool				IsAABB3DInIsometricFront(const eBounds3D & self, const eBounds3D & other);

private:

	static void				SetAABBNormal(const Uint8 entryDir, eVec2 & normal);

private:

	typedef enum {
		RIGHT	= 1,
		LEFT	= 2,
		TOP		= 4,
		BOTTOM	= 8
	} eNormalDir_t;

	
};


//***************
// eCollision::AABBContainsPoint
// returns true if the given point is within bounds
// DEBUG: includes touching
//***************
inline bool eCollision::AABBContainsPoint(const eBounds & bounds, const eVec2 & point) {
	if (point.x > bounds[1].x || point.x < bounds[0].x ||
		point.y > bounds[1].y || point.y < bounds[0].y) {
		return false;
	}
	return true;
}

//***************
// eCollision::AABBContainsAABB
// returns true if other is entirely within self
// DEBUG: includes touching
//***************
inline bool eCollision::AABBContainsAABB(const eBounds & self, const eBounds & other) {
	return (self[0][0] <= other[0][0] && self[1][0] >= other[1][0] && self[0][1] <= other[0][1] && self[1][1] >= other[1][1]);
}

//***************
// eCollision::AABBContainsPolyLine
// returns true if the given line is entirely within bounds
// DEBUG: includes touching
//***************
inline bool eCollision::AABBContainsPolyLine(const eBounds & bounds, const std::vector<eVec2> & polyLine) {
	for (auto & point : polyLine) {
		if (!AABBContainsPoint(bounds, point))
			return false;
	}
	return true;
}

//***************
// eCollision::AABBAABBTest
// returns true in the case of intersection
// DEBUG: includes touching
//***************
inline bool eCollision::AABBAABBTest(const eBounds & self, const eBounds & other) {
	if (self[1][0] < other[0][0] || self[0][0] > other[1][0]) return false;
	if (self[1][1] < other[0][1] || self[0][1] > other[1][1]) return false;
	return true;
}

//***************
// eCollision::SetAABBNormal
//***************
inline void eCollision::SetAABBNormal(const Uint8 entryDir, eVec2 & normal) {
	static const eVec2 topRight		= eVec2(0.707f, -0.707f);
	static const eVec2 bottomRight	= vec2_one * 0.707f;
	static const eVec2 topLeft		= vec2_one * -0.707f;
	static const eVec2 bottomLeft	= eVec2(-0.707f, 0.707f);

	switch(entryDir) {
		case RIGHT:				normal =  vec2_oneZero; return;
		case LEFT:				normal = -vec2_oneZero; return;
		case TOP:				normal = -vec2_zeroOne; return;
		case BOTTOM:			normal =  vec2_zeroOne; return;
		case TOP | RIGHT:		normal =  topRight; return;
		case TOP | LEFT:		normal =  topLeft; return;
		case BOTTOM | RIGHT:	normal =  bottomRight; return;
		case BOTTOM | LEFT:		normal =  bottomLeft; return;
		default:				normal =  vec2_zero; return;
	}
}

#endif /* EVIL_COLLISION_H */