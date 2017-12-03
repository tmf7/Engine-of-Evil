#include "GameObject.h"
#include "Game.h"

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(const eGameObject & other) {
	renderImage			= other.renderImage			? std::make_unique<eRenderImage>(*other.renderImage) : nullptr;
	animationController	= other.animationController ? std::make_unique<eAnimationController>(*other.animationController) : nullptr;
	collisionModel		= other.collisionModel		? std::make_unique<eCollisionModel>(*other.collisionModel) : nullptr;
	movementPlanner		= other.movementPlanner		? std::make_unique<eMovementPlanner>(*other.movementPlanner) : nullptr;
	UpdateComponentsOwner();
}

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(eGameObject && other)
	: renderImage(std::move(other.renderImage)),
	  animationController(std::move(other.animationController)),
	  collisionModel(std::move(other.collisionModel)),
	  movementPlanner(std::move(other.movementPlanner)) {
	UpdateComponentsOwner();
}

//**************
// eGameObject::operator=
//**************
eGameObject & eGameObject::operator=(eGameObject other) {
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
	oldWorldLayer = worldLayer;
	worldLayer = layer;
}

//*************
// eGameObject::SetWorldLayer
// calculates the layer with a minimum-z position
// closest to param zPosition and assigns it to worldLayer
//*************
void eGameObject::SetWorldLayer(float zPosition) {
	oldWorldLayer = worldLayer;
	worldLayer = game.GetMap().TileMap().LayerFromZPosition(eMath::NearestInt(zPosition));
}

//*************
// eGameObject::UpdateComponents
// TODO: UpdateComponents should be hidden from users... private w/eGame as a friend?
//*************
void eGameObject::UpdateComponents() {
	if (movementPlanner != nullptr)
		movementPlanner->Update();
	
	if (collisionModel != nullptr)
		collisionModel->Update();

	if (animationController != nullptr)
		animationController->Update();
	else if (renderImage != nullptr)
		renderImage->Update();
}