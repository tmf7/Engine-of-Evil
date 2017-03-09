#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Definitions.h"
#include "Bounds.h"
#include "Box.h"

class eGridCell;

//*********************************************
//			eCollisionModel
// used for movement and collision detection
//*********************************************
class eCollisionModel {
public:

								eCollisionModel();
								eCollisionModel(const eVec2 & origin, const eVec2 & velocity, const eBounds & bounds);
								~eCollisionModel();

	void						SetOrigin(const eVec2 & point);	
	const eVec2 &				Origin() const;
	void						UpdateOrigin();
	eVec2						GetOriginDelta() const;
	eBounds &					LocalBounds();
	eBounds &					AbsBounds();
	const eBounds &				LocalBounds() const;
	const eBounds &				AbsBounds() const;
	eVec2 &						Velocity();
	bool						IsActive() const;
	void						SetActive(bool active);

private:

	eBounds						localBounds;			// using model coordinates
	eBounds						absBounds;				// using world coordinates		
	eVec2						origin;					// using world coordinates
	eVec2						oldOrigin;				// for use with collision response
	eVec2						velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>	areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool						active;					// whether this participates in dynamic or kinematic collision detection

//	eEntity *					owner;					// entity using this collision model, 
														// FIXME(~): not all users are eEntity, eg eCamera and eTile

private:

	void						ClearAreas();
	void						UpdateAreas();
};

//*************
// eCollisionModel::eCollisionModel
//************
inline eCollisionModel::eCollisionModel() {
}

//*************
// eCollisionModel::eCollisionModel
// TODO: pass in initialization arguments for origin and AABB size
// because collisionModels for eTiles will have velocity == vec2_zero; (for example)
//************
inline eCollisionModel::eCollisionModel(const eVec2 & origin, const eVec2 & velocity, const eBounds & bounds)
	: velocity (velocity),
	  localBounds(bounds),
	  origin(vec2_zero) {
	SetOrigin(origin);
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
	origin += velocity;// * game.GetFixedTime();	// FIXME: defined outside this header
	absBounds = localBounds + origin;
	if (active)
		UpdateAreas();
}

//*************
// eCollisionModel::SetOrigin
//*************
inline void eCollisionModel::SetOrigin(const eVec2 & point) {
	oldOrigin = origin;
	origin = point;
	absBounds = localBounds + origin;
	if (active)
		UpdateAreas();
}

//*************
// eCollisionModel::Origin
// world-position of the collision center
// DEBUG: equivalent to absBouds.Center()
//*************
inline const eVec2 & eCollisionModel::Origin() const {
	return origin;
}

//*************
// eCollisionModel::GetOriginDelta
//*************
inline eVec2 eCollisionModel::GetOriginDelta() const {
	return origin - oldOrigin;
}

//*************
// eCollisionModel::LocalBounds
// model coordinate axis-aligned bounding box
// with center at 0,0
//*************
inline eBounds & eCollisionModel::LocalBounds() {
	return localBounds;
}
//*************
// eCollisionModel::AbsBounds
// globally positioned axis-aligned bounding box
//*************
inline eBounds & eCollisionModel::AbsBounds() {
	return absBounds;
}

//*************
// eCollisionModel::LocalBounds
// model coordinate axis-aligned bounding box
// with center at 0,0
//*************
inline const eBounds & eCollisionModel::LocalBounds() const {
	return localBounds;
}
//*************
// eCollisionModel::AbsBounds
// globally positioned axis-aligned bounding box
//*************
inline const eBounds & eCollisionModel::AbsBounds() const {
	return absBounds;
}

//*************
// eCollisionModel::Velocity
//*************
inline eVec2 & eCollisionModel::Velocity() {
	return velocity;
}

//*************
// eCollisionModel::IsActive
//*************
inline bool eCollisionModel::IsActive() const {
	return active;
}

//*************
// eCollisionModel::SetActive
//*************
inline void eCollisionModel::SetActive(bool active) {
	this->active = active;
}

#endif /* EVIL_COLLISION_MODEL_H */

