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
#include "GameObject.h"
#include "Map.h"

using namespace evil;

ECLASS_DEFINITION(eClass, eGameObject)

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(const eGameObject & other) 
	: orthoOrigin(other.orthoOrigin),
	  worldLayer(other.worldLayer),
	  isStatic(other.isStatic),
	  zPosition(other.zPosition) {

	for (auto && component : other.components) {
		if (component != nullptr)
			components.emplace_back(component->GetCopy());
	}
	UpdateComponentsOwner();
}

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(eGameObject && other)
	: orthoOrigin(std::move(other.orthoOrigin)),
	  worldLayer(other.worldLayer),
	  isStatic(other.isStatic),
	  zPosition(other.zPosition) {

	for (auto && component : other.components) {
		if (component != nullptr)
			components.emplace_back(std::move(component));
	}
	UpdateComponentsOwner();
}

//**************
// eGameObject::operator=
//**************
eGameObject & eGameObject::operator=(eGameObject other) {
	std::swap(orthoOrigin, other.orthoOrigin);
	std::swap(worldLayer, other.worldLayer);
	std::swap(isStatic, other.isStatic);
	std::swap(zPosition, other.zPosition);
	std::swap(components, other.components);
	UpdateComponentsOwner();
    return *this;
}

//**************
// eGameObject::UpdateComponentsOwner
// ensures the newly constructed *this has components with back-pointers to this
// and not the old eGameObject moved or copied from
//**************
void eGameObject::UpdateComponentsOwner(){
	for (auto && component : components) {
		if (component != nullptr)
			component->SetOwner(this);
	}
}

//*************
// eGameObject::SetWorldLayer
// directly sets the worldLayer
//*************
void eGameObject::SetWorldLayer(Uint32 layer) {
	worldLayer = layer;
}

//*************
// eGameObject::SetWorldLayer
// calculates the layer with a minimum-z position
// closest to param zPosition and assigns it to worldLayer
//*************
void eGameObject::SetWorldLayer(float zPosition) {
	worldLayer = map->TileMap().LayerFromZPosition(eMath::NearestInt(zPosition));
}

//*************
// eGameObject::SetZPosition
// sets the fluid height of *this above/below the map
// DEBUG: also updates the worldLayer using eGameObject::SetWorldLayer
//*************
void eGameObject::SetZPosition(float newZPosition) {
	zPosition = newZPosition;
	SetWorldLayer(zPosition);
}

//*************
// eGameObject::UpdateComponents
// TODO(?): should UpdateComponents be hidden from users?
//*************
void eGameObject::UpdateComponents() {
	for (auto && component : components) {
		if (component != nullptr)
			component->Update();
	}
}

//*************
// eGameObject::AddAnimationController
// returns true if an eAnimationController has been added to *this
// returns false if not because the filename is invalid, the file is unreadable, 
// DEBUG: call [AddRenderImageBase|AddRenderImageIsometric] before calling this, because it depends on an eRenderImage-derived object
//*************
bool eGameObject::AddAnimationController( const std::string & animationControllerFilename ) {
	std::shared_ptr<eAnimationController> prefabAnimationController = nullptr;
	if ( !game->GetAnimationControllerManager().LoadAndGet( animationControllerFilename.c_str(), prefabAnimationController ) )
		return false;

	animationController = std::make_unique< eAnimationController >( this, *prefabAnimationController );
	return true;
}

//*************
// eGameObject::AddMovementPlanner
// DEBUG: ideally call AddCollisionModel before calling this, because it depends on an eCollisionModel object
//*************
void eGameObject::AddMovementPlanner( float movementSpeed ) {
	movementPlanner = std::make_unique< eMovementPlanner >( this, movementSpeed );
}