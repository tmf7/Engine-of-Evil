#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Definitions.h"
#include "Collision.h"
#include "Component.h"

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
class eCollisionModel : public eComponent {
public:

										eCollisionModel(eGameObject * owner);
	virtual								~eCollisionModel() override;

	void								SetOrigin(const eVec2 & point);	
	const eVec2 &						Origin() const;
	void								Update();
	eVec2								GetOriginDelta() const;
	const eVec2 &						Offset() const;
	void								SetOffset(const eVec2 & newOffset);
	eBounds &							LocalBounds();
	eBounds &							AbsBounds();
	const eBounds &						LocalBounds() const;
	const eBounds &						AbsBounds() const;
	void								SetVelocity(const eVec2 & newVelocity);
	const eVec2 &						GetVelocity() const;
	const eVec2 &						GetOldVelocity() const;
	bool								IsActive() const;
	void								SetActive(bool active);
	const std::vector<eGridCell *> &	Areas() const;
	bool								FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const;

	virtual int							GetClassType() const override { return CLASS_COLLISIONMODEL; }

private:

	void								ClearAreas();
	void								UpdateAreas();
	void								AvoidCollisionSlide();
	void								AvoidCollisionCorrection();

private:

	eBounds								localBounds;			// using model coordinates
	eBounds								absBounds;				// using world coordinates	
	eVec2								origin;					// using world coordinates
	eVec2								oldOrigin;				// for use with collision response
	eVec2								orthoOriginOffset;		// offset from (eGameObject)owner::orthoOrigin (default: (0,0))
	eVec2								oldVelocity;			// velocity of the prior frame
	eVec2								velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>			areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool								active;					// whether this participates in (dynamic or kinematic) collision detection

};

//*************
// eCollisionModel::eCollisionModel
//*************
inline eCollisionModel::eCollisionModel(eGameObject * owner) {
	this->owner = owner;
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
// eCollisionModel::Offset
// x and y distance from owner::orthoOrigin
//*************
inline const eVec2 & eCollisionModel::Offset() const {
	return orthoOriginOffset;
}

//*************
// eCollisionModel::SetOffset
// sets the x and y distance from owner::orthoOrigin
//*************
inline void eCollisionModel::SetOffset(const eVec2 & newOffset) {
	orthoOriginOffset = newOffset;
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
// eCollisionModel::SetVelocity
//*************
inline void eCollisionModel::SetVelocity(const eVec2 & newVelocity) {
	oldVelocity = velocity;
	velocity = newVelocity;
}

//*************
// eCollisionModel::GetVelocity
//*************
inline const eVec2 & eCollisionModel::GetVelocity() const {
	return velocity;
}

//*************
// eCollisionModel::GetOldVelocity
//*************
inline const eVec2 & eCollisionModel::GetOldVelocity() const {
	return oldVelocity;
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

