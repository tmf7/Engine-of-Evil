#include "GameObject.h"

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(const eGameObject & other) {
	renderImage			= other.renderImage			? std::make_unique<eRenderImage>(*other.renderImage) : nullptr;
	animationController	= other.animationController ? std::make_unique<eAnimationController>(*other.animationController) : nullptr;
	collisionModel		= other.collisionModel		? std::make_unique<eCollisionModel>(*other.collisionModel) : nullptr;
	movementPlanner		= other.movementPlanner		? std::make_unique<eMovementPlanner>(*other.movementPlanner) : nullptr;
}

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(eGameObject && other)
	: renderImage(std::move(other.renderImage)),
	  animationController(std::move(other.animationController)),
	  collisionModel(std::move(other.collisionModel)),
	  movementPlanner(std::move(other.movementPlanner)) {

}

//**************
// eGameObject::operator=
//**************
eGameObject & eGameObject::operator=(eGameObject other) {
	std::swap(renderImage, other.renderImage);
	std::swap(animationController, other.animationController);
	std::swap(collisionModel, other.collisionModel);
	std::swap(movementPlanner, other.movementPlanner);
    return *this;
}