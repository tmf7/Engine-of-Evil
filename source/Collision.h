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
	static eBounds			GetBroadPhaseBounds(eCollisionModel & self);
	static bool				AABBContainsPoint(const eBounds & bounds, const eVec2 & point);
	static bool				AABBAABBTest(const eBounds & a, const eBounds & b);
	static bool				IsAABB3DInIsometricFront(const eBounds3D & self, const eBounds3D & other);
	static Collision_t		MovingAABBAABBTest(eCollisionModel & self, eCollisionModel & other);
	static bool				ForwardCollisionTest(eCollisionModel & self, std::vector<Collision_t> & collisions);
};

#endif /* EVIL_COLLISION_H */