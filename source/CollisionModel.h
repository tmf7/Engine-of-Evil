#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Definitions.h"
#include "Collision.h"

class eEntity;
class eGridCell;

typedef struct Collision_s Collision_t;

//*********************************************
//			eCollisionModel
// used for movement and collision detection
// TODO: either templatize this class, or
// simplify and derive from it for different
// colliders (eBounds, eBox, etc) to be used
// TODO: maintain a bounding volume heierarchy using multiple colliders
//*********************************************
class eCollisionModel : public eClass {
public:

										~eCollisionModel();
										eCollisionModel() = default;
										eCollisionModel(const eCollisionModel & other) = default;
										eCollisionModel(eCollisionModel && other) = default;
	eCollisionModel &					operator=(const eCollisionModel & other) = default;
	eCollisionModel &					operator=(eCollisionModel && other) = default;

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
	void								SetOwner(eClass * newOwner);
	const eClass *						Owner() const;
	eClass *							Owner();
	bool								FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const;

	virtual int							GetClassType() const override { return CLASS_COLLISIONMODEL; }

private:

	void								ClearAreas();
	void								UpdateAreas();
	void								AvoidCollisionSlide();
	void								AvoidCollisionCorrection();

private:

	eClass *							owner = nullptr;		// eClass (FIXME: eGameObject) using this collision model
																// FIXME/BUG: std::shared_ptr creates a loop and artificial memory leak because
																// neither this nor its owner can be destroyed
																// raw pointer is okay because *this doesn't outlive its owner 
																// (and all ::CollisionModel() shared_ptr copies are fn-scoped)
																// FIXME/BUG: *owner may move in memory because of std::vector resizing
																// SOLTUION(?): use observer pattern w/std::weak_ptr (owner as the subject, *this as observer)
																// SOLUTION(?): update this->owner whenever ANY of owners ctor/assignments are called [cheaper?]
																// which means creating rule of 5 for everything with an eCollisionModel (and anything that has an "owner" backpointer, eg: eMovement)
																// FIXME/BUG(!): eEntity::Spawn creates a copy of a prefab and initializes it, 
																// so owner == &prefabEntity not the one in use
																// SOLUTION(~): assign this->owner in eEntity::Spawn, and leave eCamera and eTile collisionModel owners nullptr

	eBounds								localBounds;			// using model coordinates
	eBounds								absBounds;				// using world coordinates	
	eVec2								origin;					// using world coordinates
	eVec2								oldOrigin;				// for use with collision response
	eVec2								velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>			areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool								active;					// whether this participates in dynamic or kinematic collision detection

};

//*************
// eCollisionModel::~eCollisionModel
//************
inline eCollisionModel::~eCollisionModel() {
	ClearAreas();
}

//*************
// eCollisionModel::SetOwner
//************
inline void eCollisionModel::SetOwner(eClass * newOwner) {
	owner = newOwner;
}

//*************
// eCollisionModel::Owner
//************
inline const eClass * eCollisionModel::Owner() const {
	return owner;
}

//*************
// eCollisionModel::Owner
//************
inline eClass * eCollisionModel::Owner() {
	return owner;
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
// eCollisionModel::Areas
//*************
inline const std::vector<eGridCell *> & eCollisionModel::Areas() const {
	return areas;
}

#endif /* EVIL_COLLISION_MODEL_H */

