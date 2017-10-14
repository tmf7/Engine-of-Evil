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
										~eCollisionModel();

	void								SetOrigin(const eVec2 & point);	
	const eVec2 &						Origin() const;
	void								UpdateOrigin();
	eVec2								GetOriginDelta() const;
	eBounds &							LocalBounds();
	eBounds &							AbsBounds();
	const eBounds &						LocalBounds() const;
	const eBounds &						AbsBounds() const;
	eVec2 &								Velocity();
	const eVec2 &						Velocity() const;
	bool								IsActive() const;
	void								SetActive(bool active);
	const std::vector<eGridCell *> &	Areas() const;

private:

	eBounds						localBounds;			// using model coordinates
	eBounds						absBounds;				// using world coordinates	
	
//	std::shared_ptr<eCollider>	absBounds;		// TODO: test abstracted collider types...
//												// TODO: how many colliders can an eEntity be associated with...collision world...disconnected from entities?! (owners)
	
	eVec2						origin;					// using world coordinates
	eVec2						oldOrigin;				// for use with collision response
	eVec2						velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>	areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool						active;					// whether this participates in dynamic or kinematic collision detection

//	eEntity *					owner;					// entity using this collision model, 
														// FIXME(~): not all users are eEntity, eg eCamera and eTile (they don't need collision models, maybe? certainly not eTile)
														// or just make a base eGameObject interface that everyone inherits from.~

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

//	TODO: if getting rid of localBounds (to just calculate it when needed based on origin and absBounds)
//	absBounds += (origin - oldOrigin);		// FIXME: or translation = velocity * deltaTime; (then apply one translation to origin and absBounds)
// TODO: OR, get rid of velocity as well, and defer that to a physics/rigidbody class (let eAI use a *placeholder* velocity in the meantime)
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
// DEBUG: NOT equivalent to absBounds.Center() if localBounds isn't centered on (0,0)
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
// eCollisionModel::Velocity
//*************
inline const eVec2 & eCollisionModel::Velocity() const {
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

//*************
// eCollisionModel::SetActive
//*************
inline const std::vector<eGridCell *> & eCollisionModel::Areas() const {
	return areas;
}

#endif /* EVIL_COLLISION_MODEL_H */

