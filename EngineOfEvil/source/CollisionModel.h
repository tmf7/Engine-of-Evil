/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
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
	virtual										~eCollisionModel() override;

	void										SetOrigin(const eVec2 & newOrigin);	
	const eVec2 &								Center() const;
	eVec2										GetOriginDelta() const;
	const eVec2 &								Offset() const;
	void										SetOffset(const eVec2 & newOffset);
	void										SetLocalBounds(const eBounds & newLocalBounds);
	const eBounds &								LocalBounds() const;
	const eBounds &								AbsBounds() const;
	void										SetVelocity(const eVec2 & newVelocity);
	const eVec2 &								GetVelocity() const;
	const eVec2 &								GetOldVelocity() const;
	bool										IsActive() const;
	void										SetActive(bool active);
	const std::vector<eGridCell *> &			Areas() const;
	bool										FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const;

	virtual void								Update() override;
	virtual std::unique_ptr<eComponent>			GetCopy() const override					{ return std::make_unique<eCollisionModel>(*this); }
	virtual int									GetClassType() const override				{ return CLASS_COLLISIONMODEL; }
	virtual bool								IsClassType(int classType) const override	{ 
													if(classType == CLASS_COLLISIONMODEL) 
														return true; 
													return eComponent::IsClassType(classType); 
												}

private:

	void										ClearAreas();
	void										UpdateAreas();
	void										AvoidCollisionSlide();
	void										AvoidCollisionCorrection();

private:

	eBounds										localBounds;			// using model coordinates
	eBounds										absBounds;				// using world coordinates	
	eVec2										origin;					// using world coordinates
	eVec2										oldOrigin;				// for use with collision response
	eVec2										center;					// cached absBounds.Center() for faster updates
	eVec2										orthoOriginOffset;		// offset from (eGameObject)owner::orthoOrigin (default: (0,0))
	eVec2										oldVelocity;			// velocity of the prior frame
	eVec2										velocity;				// DEBUG: never normalized, only rotated and scaled
	std::vector<eGridCell *>					areas;					// currently occupied tileMap indexes (between 1 and 4)
	bool										active;					// whether this participates in (dynamic or kinematic) collision detection

};

//*************
// eCollisionModel::eCollisionModel
//*************
inline eCollisionModel::eCollisionModel(eGameObject * owner) {
	this->owner = owner;
}

//*************
// eCollisionModel::Center
// world-position of the collision center
//*************
inline const eVec2 & eCollisionModel::Center() const {
	return center;
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
// eCollisionModel::SetLocalBounds
// model coordinate axis-aligned bounding box
//*************
inline void eCollisionModel::SetLocalBounds(const eBounds & newLocalBounds) {
	localBounds = newLocalBounds;
}


//*************
// eCollisionModel::LocalBounds
// model coordinate axis-aligned bounding box
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

