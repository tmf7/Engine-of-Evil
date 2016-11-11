#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Bounds.h"
#include "Vector.h"

// TODO: create a potential newOrigin, test it for collision detection, the perform collision response
// TODO: also handle basic "physics" here?

// DOOM: Entity has a physicsObject, which has a clipModel, which has a bounds and origin and 
// and because idCamera is a idEntity, that has those too

class eCollisionModel {
public:
	eCollisionModel();

private:

	eBounds				localBounds;			// using local coordinates
	eBounds				absBounds;				// using map coordinates
	eVec2				origin;
	eVec2				oldOrigin;				// for use with collision response

	// TODO: should these be in a separate physics class?
	eVec2				velocity;
	float				speed;
	float				maxSpeed;
};

inline eCollisionModel::eCollisionModel() {
}

#endif /* EVIL_COLLISION_MODEL_H */

