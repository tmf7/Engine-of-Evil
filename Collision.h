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

	static void				GetAreaContents(const eVec2 & centerPoint, std::vector<eCollisionModel *> & areaContents);
	static eBounds			GetBroadPhaseBounds(const eCollisionModel & self);
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static float			MovingAABBAABBTest(const eCollisionModel & a, const eCollisionModel & b, eVec2 & collisionNormal);
	static bool				ForwardCollisionTest(const eCollisionModel & self, const std::vector<eCollisionModel *> & areaContents, std::vector<Collision_t> & collisions);
};

#endif /* EVIL_COLLISION_H */