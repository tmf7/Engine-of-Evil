#ifndef EVIL_COLLISION_H
#define EVIL_COLLISION_H

#include "Definitions.h"

class eVec2;

// Collision_t
typedef struct Collision_s {
	eVec2				normal;			// surface normal of collided object 
	float				fraction;		// fraction along a movement to first contact
	eCollisionModel *	owner;			// collided object
} Collision_t;


//************************************
//			eCollision
// utility class for collision tests
//***********************************
class eCollision {
public:

	static eVec2			GetCollisionNormal(eCollisionModel & self, const eCollisionModel & other);
	static void				GetIsometricAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells);
	static void				GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells);
	static eBounds			GetBroadPhaseBounds(eCollisionModel & self);
	static bool				AABBContainsPoint(const eBounds & bounds, const eVec2 & point);
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static Collision_t		MovingAABBAABBTest(eCollisionModel & self, eCollisionModel & other);
	static bool				ForwardCollisionTest(eCollisionModel & self, const std::vector<eGridCell *> & areaCells, std::vector<Collision_t> & collisions);
};

#endif /* EVIL_COLLISION_H */