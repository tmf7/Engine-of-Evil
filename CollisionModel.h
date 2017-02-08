#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Definitions.h"
#include "Bounds.h"

class eTile;

// DOOM: Entity has a physicsObject, which has a clipModel, which has a bounds and origin and 
// and because idCamera is a idEntity, that has those too

//*********************************************
//			eCollisionModel
// used for movement and collision detection

// TODO: make the renderImage_t more versitle (and less expensive?) such that each eEntity already has a renderImage_t
// being updated, and a SHARED_POINTER of which is sent to the renderpool (maybe???)
// ...each renderImage_t would have a bounds (or cliprect?) and origin (like eCollisionModel) such that they'd
// have to be ****synchronized WELL*** to produce accurate collision and visual movement
// ....or.....make the renderImage_t depend on the eCollisionModel directly....like...give it one....?
// so there'd only be one origin and bounds to worry about....EXCEPT sprite images start drawing from the top-left (THAT origin) not center.
// NOTE: that wouldn't change the debug draws at all (they'd still be an overlay)
// NOTE: would it affect non-constText? a pointer to a renderimage that exists only for a moment....yup, should be fine using shared_ptr
//*********************************************
class eCollisionModel {
public:

							eCollisionModel();
							~eCollisionModel();

	void					SetOrigin(const eVec2 & point);	
	const eVec2 &			Origin() const;
	void					UpdateOrigin();
	const eBounds &			Bounds() const;	
	const eVec2 &			Velocity() const;

private:

	eBounds					localBounds;			// using local coordinates
	eBounds					absBounds;				// using world coordinates		

	// FIXME(?): absBounds' center is already the origin, though to query it means to perform a calulation each time instead of caching it
	// and using the origin to translate the localBounds to equal the absBounds ignores the use of eBounds::TranslateSelf(velocity * deltaTime)
	// so the question becomes: when do I use localBounds really? I dont. it can all be replaced with worldBounds or absBounds or just bounds

	eVec2					origin;					// using world coordinates
	eVec2					oldOrigin;				// for use with collision response
	eVec2					velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eTile *>	areas;					// currently occupied tileMap indexes (between 1 and 4)

private:

	void					ClearAreas();
	void					UpdateAreas();
};

//*************
// eCollisionModel::eCollisionModel
// TODO: pass in initialization arguments for origin and localBounds
// because collisionModels for eTiles will have velocity == vec2_zero; (for example)
//************
inline eCollisionModel::eCollisionModel() {
	velocity = vec2_oneZero * 10.0f;
	UpdateAreas();
}

//*************
// eCollisionModel::~eCollisionModel
//************
inline eCollisionModel::~eCollisionModel() {
	ClearAreas();
}

//*************
// eCollisionModel::UpdateOrigin
//*************
inline void eCollisionModel::UpdateOrigin() {
	oldOrigin = origin;
	origin += velocity;
	absBounds = localBounds + origin;
	UpdateAreas();
}

//*************
// eCollisionModel::SetOrigin
//*************
inline void eCollisionModel::SetOrigin(const eVec2 & point) {
	oldOrigin = point;
	origin = point;
	absBounds = localBounds + origin;
	UpdateAreas();
}

//*************
// eCollisionModel::Origin
//*************
inline const eVec2 & eCollisionModel::Origin() const {
	return origin;
}

//*************
// eCollisionModel::Bounds
//*************
inline const eBounds & eCollisionModel::Bounds() const {
	return absBounds;
}

//*************
// eCollisionModel::Velocity
//*************
inline const eVec2 & eCollisionModel::Velocity() const {
	return velocity;
}

#endif /* EVIL_COLLISION_MODEL_H */

