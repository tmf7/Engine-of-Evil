#ifndef EVIL_COLLISION_MODEL_H
#define EVIL_COLLISION_MODEL_H

#include "Definitions.h"
#include "Collision.h"

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
										eCollisionModel(const std::shared_ptr<eClass> & owner);
										eCollisionModel() = delete;							// DEBUG: was default
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
	const eClass &						Owner() const;
	eClass &							Owner();
	bool								FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const;

	virtual int							GetClassType() const override { return CLASS_COLLISIONMODEL; }

private:

	void								ClearAreas();
	void								UpdateAreas();
	void								AvoidCollisionSlide();
	void								AvoidCollisionCorrection();

private:

	std::shared_ptr<eClass>				owner;					// eClass (FIXME: eGameObject) using this collision model

	eBounds								localBounds;			// using model coordinates
	eBounds								absBounds;				// using world coordinates	
	eVec2								origin;					// using world coordinates
	eVec2								oldOrigin;				// for use with collision response
	eVec2								velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>			areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool								active;					// whether this participates in dynamic or kinematic collision detection

};

//*************
// eCollisionModel::eCollisionModel
//************
inline eCollisionModel::eCollisionModel(const std::shared_ptr<eClass> & owner) 
	: owner(owner) {
}

//*************
// eCollisionModel::~eCollisionModel
//************
inline eCollisionModel::~eCollisionModel() {
	ClearAreas();
}

//*************
// eCollisionModel::Owner
//************
inline const eClass & eCollisionModel::Owner() const {
	return *owner;
}

//*************
// eCollisionModel::Owner
//************
inline eClass & eCollisionModel::Owner() {
	return *owner;
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

