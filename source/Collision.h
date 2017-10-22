#ifndef EVIL_COLLISION_H
#define EVIL_COLLISION_H

#include "Definitions.h"

class eVec2;
class eBounds3D;
extern const float noCollisionFraction;

// Collision_t
typedef struct Collision_s {
	eVec2				normal;								// surface normal of collided object (default ctor: x = 0.0f, y = 0.0f)
	float				fraction = noCollisionFraction;		// fraction along a movement to first contact
	eCollisionModel *	owner = nullptr;					// collided object
} Collision_t;


//************************************
//			eCollision
// utility class for collision tests
//***********************************
class eCollision {
public:

	static bool				OBBOBBTest(const eBox & a, const eBox & b);
	static void				GetCollisionNormal(const eCollisionModel & self, const eCollisionModel & other, Collision_t & collision);
	static void				GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells);
	static eBounds			GetBroadPhaseBounds(const eCollisionModel & self);
	static bool				AABBContainsPoint(const eBounds & bounds, const eVec2 & point);
	static bool				AABBContainsAABB(const eBounds & self, const eBounds & other);
	static bool				AABBContainsPolyLine(const eBounds & bounds, const std::vector<eVec2> & polyLine);	
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static bool				IsAABB3DInIsometricFront(const eBounds3D & self, const eBounds3D & other);
	static Collision_t		MovingAABBAABBTest(const eCollisionModel & self, eCollisionModel & other);
	static bool				ForwardCollisionTest(const eCollisionModel & self, std::vector<Collision_t> & collisions);
	static bool				RayAABBTest(const eVec2 & begin, const eVec2 & dir, const eBounds & bounds);
	static bool				LineAABBTest(const eVec2 & begin, const eVec2 & end, const eBounds & bounds);

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
// returns true if other is fully within self
// DEBUG: includes touching
//***************
inline bool eCollision::AABBContainsAABB(const eBounds & self, const eBounds & other) {
	return (self[0][0] <= other[0][0] && self[1][0] >= other[1][0] && self[0][1] <= other[0][1] && self[1][1] >= other[1][1]);
}

//***************
// eCollision::AABBContainsLine
// returns true if the given line is within bounds
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
#endif /* EVIL_COLLISION_H */