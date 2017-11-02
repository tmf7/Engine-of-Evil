#ifndef EVIL_COLLISION_H
#define EVIL_COLLISION_H

#include "Definitions.h"

class eVec2;
class eBounds3D;

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
//***********************************
class eCollision {
public:

	static bool				OBBOBBTest(const eBox & a, const eBox & b);
	static void				GetCollisionNormal(const eVec2 & point, const eBounds & bounds, eVec2 & resultNormal);
	static void				GetCollisionNormal(eBounds self, const eVec2 & dir, const float length, const eBounds & other, Collision_t & collision);
	static void				GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(const eBounds & bounds, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(const eVec2 & begin, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells);
	static bool				AABBContainsPoint(const eBounds & bounds, const eVec2 & point);
	static bool				AABBContainsAABB(const eBounds & self, const eBounds & other);
	static bool				AABBContainsPolyLine(const eBounds & bounds, const std::vector<eVec2> & polyLine);	
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static bool				SegmentAABBTest(const eVec2 & begin, const eVec2 & end, const eBounds & bounds);
	static bool				IsAABB3DInIsometricFront(const eBounds3D & self, const eBounds3D & other);
	static bool				MovingAABBAABBTest(const eBounds & self, const eVec2 & dir, const float length, const eBounds & other, float & resultFraction);
	static bool				RayAABBTest(const eVec2 & begin, const eVec2 & dir, const float length, const eBounds & bounds, float & resultFraction);
	static bool				BoxCast(std::vector<Collision_t> & collisions, const eBounds & bounds, const eVec2 & dir, const float length);
	static bool				RayCast(std::vector<Collision_t> & collisions, const eVec2 & begin, const eVec2 & dir, const float length = FLT_MAX, bool ignoreStartInCollision = true);
	static eBounds			GetBroadPhaseBounds(const eBounds & bounds, const eVec2 & dir, const float length);

private:

	static void				SetAABBNormal(const Uint8 entryDir, const eVec2 & dir, eVec2 & normal);

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
inline void eCollision::SetAABBNormal(const Uint8 entryDir, const eVec2 & dir, eVec2 & normal) {
	switch(entryDir) {
		case RIGHT:				normal =  vec2_oneZero; return;
		case LEFT:				normal = -vec2_oneZero; return;
		case TOP:				normal = -vec2_zeroOne; return;
		case BOTTOM:			normal =  vec2_zeroOne; return;
//		case RIGHT | TOP:		normal =  eVec2(0.707f, -0.707f); return;
//		case RIGHT | BOTTOM:	normal =  vec2_one * 0.707f; return;
//		case LEFT | TOP:		normal =  vec2_one * -0.707f; return;
//		case LEFT | BOTTOM:		normal =  eVec2(-0.707f, 0.707f); return;

		case RIGHT | TOP: {
			// normals: (0,-1), (1,0)
			float upDot = abs(-vec2_zeroOne * dir);
			float rightDot = abs(vec2_oneZero * dir);
			float trueDot = abs(eVec2(0.707f, -0.707f) * dir);
			normal = (abs(upDot - trueDot) > abs(rightDot - trueDot) ? -vec2_zeroOne : vec2_oneZero);
			return;
		}
		case RIGHT | BOTTOM: {
			// normals: (0,1), (1,0)
			float downDot = abs(vec2_zeroOne * dir);
			float rightDot = abs(vec2_oneZero * dir);
			float trueDot = abs(vec2_one * 0.707f * dir);
			normal = (abs(downDot - trueDot) > abs(rightDot - trueDot) ? vec2_zeroOne : vec2_oneZero);
			return;
		}
		case LEFT | TOP: {
			// normals: (0,-1), (-1,0)
			float upDot = abs(-vec2_zeroOne * dir);
			float leftDot = abs(-vec2_oneZero * dir);
			float trueDot = abs(vec2_one * -0.707f * dir);
			normal = (abs(upDot - trueDot) > abs(leftDot - trueDot) ? -vec2_zeroOne : -vec2_oneZero);
			return;
		}
		case LEFT | BOTTOM: {
			// normals: (0,1), (-1,0)
			float downDot = abs(vec2_zeroOne * dir);
			float leftDot = abs(-vec2_oneZero * dir);
			float trueDot = abs(eVec2(-0.707f, 0.707f) * dir);
			normal = (abs(downDot - trueDot) > abs(leftDot - trueDot) ? vec2_zeroOne : -vec2_oneZero);
			return;
		}
		default:				normal =  vec2_zero; return;
	}
}

#endif /* EVIL_COLLISION_H */