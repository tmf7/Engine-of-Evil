#include "Game.h"

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(const entitySpawnArgs_t & spawnArgs)
	: eResource(spawnArgs.sourceFilename, spawnArgs.prefabManagerIndex),
	  spawnedEntityID(-1) {

	if (!spawnArgs.localBounds.IsEmpty()) {
		collisionModel = std::make_unique<eCollisionModel>(this);
		collisionModel->SetLocalBounds(spawnArgs.localBounds);
		collisionModel->SetOffset(spawnArgs.colliderOffset);
		collisionModel->SetActive(spawnArgs.collisionActive);
	
		if (spawnArgs.movementSpeed)
			movementPlanner = std::make_unique<eMovementPlanner>(this, spawnArgs.movementSpeed);
	}

	if (!spawnArgs.spriteFilename.empty()) {

		// init renderImage draw order sorting
		renderImage = std::make_unique<eRenderImage>(this);
		std::shared_ptr<eImage> spriteImage = nullptr;
		if (!game.GetImageManager().LoadAndGet(spawnArgs.spriteFilename.c_str(), spriteImage))
			throw badEntityCtorException(spawnArgs.spriteFilename.c_str());

		renderImage->SetImage(spriteImage->GetManagerIndex());
		int initialSpriteFrame = 0;
		if (spawnArgs.initialSpriteFrame < 0 || spawnArgs.initialSpriteFrame > spriteImage->NumSubframes())
			initialSpriteFrame = 0;

		renderImage->SetImageFrame(initialSpriteFrame);
		renderImage->SetOffset(spawnArgs.renderImageOffset);
		renderImage->SetRenderBlockSize( spawnArgs.renderBlockSize);

		// init animationController, if any
		if (!spawnArgs.animationControllerFilename.empty()) {
			std::shared_ptr<eAnimationController> prefabAnimationController = nullptr;
			if (!game.GetAnimationControllerManager().LoadAndGet(spawnArgs.animationControllerFilename.c_str(), prefabAnimationController))
				throw badEntityCtorException(spawnArgs.animationControllerFilename.c_str());	

			animationController = std::make_unique<eAnimationController>(*prefabAnimationController);
		}
	}
}

//***************
// eEntity::Spawn
// copies a prefab eEntity and adds unique details
// FIXME: don't throw exceptions in eEntity::eEntity, just assign defaults
//***************
bool eEntity::Spawn(const eVec3 & worldPosition) {
	int spawnID = game.AddEntity(std::make_unique<eEntity>(*this));
	if (spawnID < 0)
		return false;

	auto & newEntity = game.GetEntity(spawnID);
	if (newEntity->renderImage != nullptr) {
		newEntity->SetStatic(false);
		newEntity->renderImage->SetIsSelectable(true);
	}

	newEntity->SetWorldLayer(worldPosition.z);
	newEntity->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	return true;
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw() {
	if (game.debugFlags.RENDERBLOCKS && animationController != nullptr)
		game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderImage->GetRenderBlock(), RENDERTYPE_DYNAMIC);

//		game.GetRenderer().DrawCartesianRect(lightBlueColor, renderImage->GetWorldClip(), false, RENDERTYPE_DYNAMIC);		// DEBUG: draw worldclip

	// TODO: better visual difference b/t cells occupied by both renderImage and collisionmodel
	if (game.debugFlags.GRID_OCCUPANCY) {
		if (renderImage != nullptr) {
			for (auto & cell : renderImage->Areas()) {
				game.GetRenderer().DrawIsometricRect(yellowColor, cell->AbsBounds(), RENDERTYPE_DYNAMIC);
			}
		}

		if (collisionModel != nullptr) {
			for (auto & cell : collisionModel->Areas()) {
				game.GetRenderer().DrawIsometricRect(lightBlueColor, cell->AbsBounds(), RENDERTYPE_DYNAMIC);
			}
		}
	}

	if (game.debugFlags.COLLISION && collisionModel != nullptr)
		game.GetRenderer().DrawIsometricRect(yellowColor, collisionModel->AbsBounds(), RENDERTYPE_DYNAMIC);

	if (movementPlanner != nullptr)
		movementPlanner->DebugDraw();
}