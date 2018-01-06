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

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(const eGameObject & other) 
	: orthoOrigin(other.orthoOrigin),
	  worldLayer(other.worldLayer),
	  isStatic(other.isStatic),
	  zPosition(other.zPosition) {

	// DEBUG: using std::make_unique and release to prevent leaks in the event an allocation fails
	if (other.renderImage != nullptr)			renderImage.reset(static_cast<eRenderImageBase *>(other.renderImage->GetCopy().release()));
	if (other.animationController != nullptr)	animationController.reset(static_cast<eAnimationController *>(other.animationController->GetCopy().release()));
	if (other.collisionModel != nullptr)		collisionModel.reset(static_cast<eCollisionModel *>(other.collisionModel->GetCopy().release()));
	if (other.movementPlanner != nullptr)		movementPlanner.reset(static_cast<eMovementPlanner *>(other.movementPlanner->GetCopy().release()));	
	UpdateComponentsOwner();
}

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(eGameObject && other)
	: orthoOrigin(std::move(other.orthoOrigin)),
	  worldLayer(other.worldLayer),
	  isStatic(other.isStatic),
	  zPosition(other.zPosition),
	  renderImage(std::move(other.renderImage)),
	  animationController(std::move(other.animationController)),
	  collisionModel(std::move(other.collisionModel)),
	  movementPlanner(std::move(other.movementPlanner)) {
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
	std::swap(renderImage, other.renderImage);
	std::swap(animationController, other.animationController);
	std::swap(collisionModel, other.collisionModel);
	std::swap(movementPlanner, other.movementPlanner);
	UpdateComponentsOwner();
    return *this;
}

//**************
// eGameObject::UpdateComponentsOwner
// ensures the newly constructed *this has components with back-pointers to this
// and not the old eGameObject moved or copied from
//**************
void eGameObject::UpdateComponentsOwner(){
	if (renderImage != nullptr)			renderImage->SetOwner(this);
	if (animationController != nullptr) animationController->SetOwner(this);
	if (collisionModel != nullptr)		collisionModel->SetOwner(this);
	if (movementPlanner != nullptr)		movementPlanner->SetOwner(this);
}

//*************
// eGameObject::SetOrigin
//*************
void eGameObject::SetOrigin(const eVec2 & newOrigin) {
	orthoOrigin = newOrigin;
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
// TODO(?): should UpdateComponents be hidden from users... private w/eGame as a friend?
//*************
void eGameObject::UpdateComponents() {
	if (movementPlanner != nullptr)
		movementPlanner->Update();
	
	if (collisionModel != nullptr)
		collisionModel->Update();

	if (animationController != nullptr)
		animationController->Update();

	if (renderImage != nullptr)
		renderImage->Update();
}


//*************
// eGameObject::AddRenderImageBase
// returns true if an eRenderImageBase object has been added to *this
// returns false if not because the filename is invalid, or the file is unreadable
//*************
bool eGameObject::AddRenderImageBase( const std::string & spriteFilename, int initialSpriteFrame, const eVec2 & renderImageOffset, bool isPlayerSelectable ) {
	std::shared_ptr<eImage> initialImage = nullptr;
	if (!game->GetImageManager().LoadAndGet(spriteFilename.c_str(), initialImage))
		return false;

	renderImage = std::make_unique<eRenderImageBase>(this, initialImage, initialSpriteFrame, renderImageOffset, isPlayerSelectable);
	return true;
}

//*************
// eGameObject::AddRenderImageIsometric
// returns true if an eRenderImageIsometric object has been added to *this
// returns false if not because the filename is invalid, or the file is unreadable
//*************
bool eGameObject::AddRenderImageIsometric( const std::string & spriteFilename, const eVec3 & renderBlockSize, int initialSpriteFrame, const eVec2 & renderImageOffset, bool isPlayerSelectable ) {
	std::shared_ptr<eImage> initialImage = nullptr;
	if (!game->GetImageManager().LoadAndGet(spriteFilename.c_str(), initialImage))
		return false;

	renderImage = std::make_unique<eRenderImageIsometric>(this, initialImage, renderBlockSize, initialSpriteFrame, renderImageOffset, isPlayerSelectable);
	return true;
}

//*************
// eGameObject::AddCollisionModel
// returns true if an eCollisionModel has been added to *this
// returns false if not because localBounds empty
//*************
bool eGameObject::AddCollisionModel( const eBounds & localBounds, const eVec2 & colliderOffset, bool collisionActive ) {
	if (localBounds.IsEmpty())
		return false;

	collisionModel = std::make_unique<eCollisionModel>(this, localBounds, colliderOffset, collisionActive);
	return true;
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
// returns true if a eMovementPlanner has been added to *this
// returns false if not because there is no eCollisionModel attached
// DEBUG: call AddCollisionModel before calling this, because it depends on an eCollisionModel object
//*************
bool eGameObject::AddMovementPlanner( float movementSpeed ) {
	if ( collisionModel == nullptr )
		return false;

	movementPlanner = std::make_unique< eMovementPlanner >( this, movementSpeed );
	return true;
}