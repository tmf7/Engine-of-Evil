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
// FIXME:  implement parent and children copying
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
// FIXME:  implement parent and children moving
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
// FIXME:  implement parent and children swapping
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
// eGameObject::SetParent
// FIXME: handle deleting the parent gameobject: do the children get deleted too?
// FIXME: handle disabling the parent gameobject: do the children get disabled too?
// FIXME: handle what a parent layer/tag means for its children's actions|interactions

// TODO: if the parent is nullptr when calculating the eComponent eRectTransform offset, 
// just use (0,0) and the main window size by default
//*************
void eGameObject::SetParent( eGameObject * newParent )	{
	if ( newParent == this )
		return;

	// remove *this from its current list of siblings
	if ( parent != nullptr ) {
		auto & siblings = parent->children;
		auto & self = std::find( siblings.begin(), siblings.end(), this );
		siblings.erase( self );
	}

	if ( newParent != nullptr )
		newParent->children.emplace_back( this );

	parent = newParent;

	// maintain worldspace coordinates and just change relativeOrigin
	eVec2 parentOrigin = ( parent == nullptr ? vec2_zero : parent->orthoOrigin );
	SetOrigin( orthoOrigin - parentOrigin );			
}

//*************
// eGameObject::SetOrigin
// sets origin with respect to any parent
// or directly in worldspace
//*************
void eGameObject::SetOrigin( const eVec2 & newOrigin )	{
	relativeOrigin = newOrigin;

	if ( parent != nullptr ) {
		orthoOrigin = parent->orthoOrigin + newOrigin;
	} else {
		orthoOrigin = newOrigin;
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