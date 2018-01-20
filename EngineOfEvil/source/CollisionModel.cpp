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
#include "Game.h"
#include "Map.h"
#include "CollisionModel.h"

using namespace evil;

ECLASS_DEFINITION(eComponent, eCollisionModel)
ECOMPONENT_DEFINITION(eComponent, eCollisionModel)

//*************
// eCollisionModel::eCollisionModel
//*************
eCollisionModel::eCollisionModel(eGameObject * owner, const eBounds & localBounds, const eVec2 & offset, bool isActive)
	: localBounds(localBounds),
	  ownerOriginOffset(offset),
	  active(isActive) {
	this->owner = owner;
}

//*************
// eCollisionModel::~eCollisionModel
//*************
eCollisionModel::~eCollisionModel() {
	ClearAreas();
}

//************
// eCollisionModel::VerifyAdd
// only one instance per eGameObject allowed
// TODO: add a unique identifier for the eGameObject being added to
// for more accurate debug logging
//************
bool eCollisionModel::VerifyAdd() const {
	if ( owner->GetComponent<eCollisionModel>() != nullptr ) {
		eErrorLogger::LogError( "Only one eCollisionModel allowed per eGameObject.", __FILE__, __LINE__ );
		return false;
	}

	return true;
}

//*************
// eCollisionModel::SetOrigin
//*************
void eCollisionModel::SetOrigin( const eVec2 & newOrigin ) {
	oldOrigin = origin;
	origin = newOrigin;
	absBounds = localBounds + origin + ownerOriginOffset;
	center = absBounds.Center();
}

//*************
// eCollisionModel::Update
// TODO: move velocity member to physics/rigidbody class
// TODO: use ePhysics to also set the velocity (same as eMovementPlanner) while eCollisionModel alone updates origins

// FIXME/BUG(!!!): eMovementPlanner::Update and eCollisionModel::Update order may be arbitrary, which could result
// in eCollisionModel using a velocity set by eMovementPlanner in the previous frame, and not the CURRENT velocity
// SOLUITON: this is a 1-frame lag, and doesn't affect collision detection...right?

// FIXME: if a user calls eGameObject::SetOrigin (ie: outside eCollisionModel::Update) then
// then an immdiate call to eCollision::FindApproachingCollision(collisionModel,...) will be using an out of sync absBounds
// SOLUTION: have eGameObject::SetOrigin also call eComponent::SetOrigin...or eComponent::Update
// ISSUE: not all eComponents have origins
// ISSUE: eCollisionModel::Update causes the collisionModel to move 
// (and forcing a SetVelocity(0) call may affect eMovmentPlanner,
// and saving the current velocity, setting the origin, then resetting is costly)
// ISSUE: also calling eAnimationController::Update for every eGameObject::SetOrigin causes the animation to run unnecessarily
// FIXME: the same out-of-sync worldClip issue comes up for an eRenderImageBase::SetOrigin call
// SOLUTION: get rid of public SetOrigin from eComponents and make them DRIVEN by their owner's orthoOrigin
// ISSUE: better, but still doesn't solve the issue of de-synced absBounds and worldClips
// SOLUTION: add a (virtual?) void eGameObject::SyncComponents() that calls virtual void eComponent::Sync() to allow each component their own syncronizing details
// and call eGameObject::SyncComponents after an eGameObject fn call that may de-sync it from its components.
// SOLUTION(?): re-calculate absBounds and worldClip everywhere they're used (via AbsBounds() and WorldClip())
// SOLUTION: doom runs all origins through the physics object (which is driven by ai, and drives the clipModel)
//*************
void eCollisionModel::Update() {
//	SetOrigin( owner->GetOrigin() );		// BUGFIX: eGameObject::SetOrigin outside eCollisionModel::Update puts absBounds out of sync

	if ( active )
		AvoidCollisionSlide();				// TODO: alternatively, push the collider away if it can be moved (non-static)

	SetOrigin( owner->GetOrigin() + velocity * game->GetDeltaTime() );
	owner->SetOrigin( origin );				// sync the owner and collider positions

	if ( active && origin != oldOrigin )
		UpdateAreas();
}

//***************
// eCollisionModel::ClearAreas
// removes this from all eMap::tileMap gridcells with pointers to it
// and clear this->areas gridcell pointers
//***************
void eCollisionModel::ClearAreas() {
	for (auto && cell : areas) {
		auto & contents = cell->CollisionContents();
		if (contents.empty())	// FIXME: necessary to prevent a shutdown crash using std::unordered_map::find (insead of std::find)
			continue;

		auto & index = contents.find(this);
		if (index != contents.end())
			contents.erase(index);
	}
	areas.clear();
}

//***************
// eCollisionModel::UpdateAreas
// adds this to the eMap::tileMap gridcells it overlaps
// and adds those same gridcell pointers to this->areas
// DEBUG: called whenever the collisionModel moves
// FIXME(performance): this is a heavily used function and should be optimized
// cell.contents as a hashmap only helps for large collisionModels
// (however most lists are very small, eg list of 2 would be O(n=2) for both hashmap and vector 
// because hashmap would need to hash the key then compare the values)
//***************
void eCollisionModel::UpdateAreas() {
	ClearAreas();

	auto & tileMap = owner->GetMap()->TileMap();
	auto & cell = tileMap.IndexValidated(center);
	if (cell.AbsBounds() != absBounds) {
		eCollision::GetAreaCells(owner->GetMap(), absBounds, areas);
	} else {							// BUGFIX: edge case where bounds matches its cell and winds up adding 4 areas instead of 1
		areas.emplace_back(&cell);
	}

	for (auto && cell : areas) {
		cell->CollisionContents()[this] = this;
	}
}

//***************
// eCollisionModel::AvoidCollisionCorrection
// truncates current velocity to prevent intersection with other colliders 
//***************
void eCollisionModel::AvoidCollisionCorrection() {
	if (velocity == vec2_zero)
		return;	

	Collision_t collision;
	float length = velocity.Length();
	if(eCollision::FindApproachingCollision(owner->GetMap(), absBounds, velocity / length, length, collision))
		velocity *= collision.fraction;
}

//***************
// eCollisionModel::AvoidCollisionSlide
// truncates current velocity to prevent intersection with other colliders,
// or rotates velocity to slide along a touched collider
//***************
void eCollisionModel::AvoidCollisionSlide() {
	if (velocity == vec2_zero)
		return;

	Collision_t collision;
	eVec2 collisionTangent;
	float remainingFraction = 0.0f;
	float length = velocity.Length();

	if (eCollision::FindApproachingCollision(owner->GetMap(), absBounds, velocity / length, length, collision)) {
		remainingFraction = 1.0f - collision.fraction;
		if (remainingFraction < 1.0f) {
			velocity *= collision.fraction;
		} else {
			// DEBUG: collisions sorted w/edges first for equal fractions
			collisionTangent = eVec2(-collision.normal.y, collision.normal.x);		// CCW 90 degrees
			float whichWay = collisionTangent * velocity;
			if (whichWay < 0)
				collisionTangent *= -1.0f;
		} 
	}

	if (remainingFraction == 1.0f) {
		float slide = velocity * collisionTangent;
		velocity = collisionTangent * slide;	
		if (eCollision::FindApproachingCollision(owner->GetMap(), absBounds, collisionTangent, slide, collision))
			velocity *= collision.fraction;
	}
}