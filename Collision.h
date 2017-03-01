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

	static eVec2			GetCollisionNormal(const eCollisionModel & self, const eCollisionModel & other);
	static void				GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells);
	static eBounds			GetBroadPhaseBounds(const eCollisionModel & self);
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static Collision_t		MovingAABBAABBTest(const eCollisionModel & self, eCollisionModel & other);
	static bool				ForwardCollisionTest(const eCollisionModel & self, const std::vector<eGridCell *> & areaCells, std::vector<Collision_t> & collisions);
};

#endif /* EVIL_COLLISION_H */