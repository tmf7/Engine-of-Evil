#include "Game.h"

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(const entitySpawnArgs_t & spawnArgs)
	: eResource(spawnArgs.sourceFilename, spawnArgs.prefabManagerIndex),
	  imageColliderOffset(spawnArgs.imageColliderOffset),
	  spawnedEntityID(-1) {

	if (!spawnArgs.localBounds.IsEmpty()) {
		collisionModel = std::make_unique<eCollisionModel>(this);
		collisionModel->LocalBounds() = spawnArgs.localBounds;
		collisionModel->SetActive(spawnArgs.collisionActive);
	
		if (spawnArgs.movementSpeed) {
			movementPlanner = std::make_unique<eMovementPlanner>(this, spawnArgs.movementSpeed);
		}
	}

	// init renderImage for draw order sorting
	if (!spawnArgs.spriteFilename.empty()) {
		renderImage = std::make_unique<eRenderImage>(this);
		animationController = std::make_unique<eAnimationController>(this);	// TODO: animationController initialization should be just this one line
		std::shared_ptr<eImage> spriteImage = nullptr;
		if (!game.GetImageManager().LoadAndGet(spawnArgs.spriteFilename.c_str(), spriteImage))
			throw badEntityCtorException(spawnArgs.spriteFilename.c_str());	

//		animationController->SetImage(spriteImage);
		eVec3 blockMins = (eVec3)spawnArgs.localBounds[0];
		renderImage->RenderBlock() = eBounds3D(blockMins, blockMins + spawnArgs.renderBlockSize);
	}
}

//***************
// eEntity::Spawn
// copies a prefab eEntity and adds unique details
// TODO: position via a single stack eTransform, not the eCollisionModel, or eRenderImage, or eAnimationController
//***************
bool eEntity::Spawn(const int entityPrefabIndex, const eVec3 & worldPosition /*, const eVec2 & facingDir*/) {
	auto & prefabEntity = game.GetEntityPrefabManager().Get(entityPrefabIndex);
	if (!prefabEntity->IsValid())
		return false;

	int spawnID = game.NumEntities();
	try {
		game.AddEntity(std::make_shared<eEntity>(*prefabEntity));
		auto & newEntity = game.GetEntity(spawnID);
		newEntity->spawnedEntityID = spawnID;			// FIXME/BUG(!): game.RemoveEntity(entityID) will invalidate all indexes above entityID
														// SOLUTION: make game.entities a HashTable (which also partially solves the movementPlanner &owner update)
														// SOLUTION(?): or just rehash the current eHashIndex (that uses spawnIDs instead of names)
														// TODO: the same goes for ImageManager and EntityPrefabManger's HashIndexes into std::vectors
		

		if (newEntity->animationController != nullptr) {
			newEntity->IsStatic() = false;
			newEntity->renderImage->SetIsSelectable(true);
			newEntity->renderImage->RenderBlock() += worldPosition;

			// FIXME: remove renderImage dependency on the collisionModel (and vis versa)
			if (newEntity->collisionModel != nullptr)
				newEntity->collisionModel->SetOrigin(eVec2(worldPosition.x, worldPosition.y));

			// TODO: eMovementPlanner may be opposite facing, and eEntity may not have a eRenderImage, so only eAnimationController cares about facing
//			if (newEntity->animationController != nullptr)	
//				newEntity->animationController->SetFacingDirection(facingDir);		

			newEntity->UpdateRenderImageDisplay();
			newEntity->UpdateRenderImageOrigin();
		}
		return true;
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (" + std::to_string(spawnID) + ") spawn failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
}

//***************
// eEntity::Think
//***************
void eEntity::Think() {
	if (movementPlanner != nullptr)
		movementPlanner->Update();

	// TODO: make eEntity.renderImage manipulation part of eAnimationController
	// then just call animationController->Update(); here 
	if (animationController != nullptr) {
		UpdateRenderImageDisplay();
		UpdateRenderImageOrigin();
	}
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw() {
	if (game.debugFlags.RENDERBLOCKS && animationController != nullptr)
		game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderImage->RenderBlock(), RENDERTYPE_DYNAMIC);

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

//*************
// eEntity::UpdateRenderImageOrigin
// backend collision occurs on a 2D top-down grid
// UpdateRenderImageOrigin ensures only the visuals are isometric
//*************
void eEntity::UpdateRenderImageOrigin() {
	eVec2 renderImageOrigin = collisionModel->AbsBounds()[0];	// FIXME(later): eTile::renderImage::origin is unmoving in iso. world space (regardless of collision)
																// SOLUTION: treat all renderImage-collisionModel relations the same
																// everthing can have a Transform...position, orientation, scale
																// then collisionModels have origins at their center w/offset from the transform
																// and renderImage_ts have origins at their top-left corner w/ offset from the transform
	eMath::CartesianToIsometric(renderImageOrigin.x, renderImageOrigin.y);
	renderImageOrigin += imageColliderOffset;
	renderImage->SetOrigin(renderImageOrigin);
}

//*************
// eEntity::UpdateRenderImageDisplay
// TODO: uses the animationController to set the animation image and frame data
// TODO: move this to eEntity::Think, and just call animationController.Update();
//*************
void eEntity::UpdateRenderImageDisplay() {
	animationController->Update();

// FREEHILL BEGIN 3d topological sort
	// DEBUG: renderBlock and collisionModel currently designed to align, while offsetting renderImage.origin instead
	eVec2 collisionMins = collisionModel->AbsBounds()[0];
	eVec3 renderBlockMins = renderImage->RenderBlock()[0];
	renderImage->RenderBlock() += eVec3(collisionMins.x - renderBlockMins.x, collisionMins.y - renderBlockMins.y , 0.0f);
// FREEHILL END 3d topological sort

	// TODO: eEntity layer unused, but set for possible future use
	// EG: with a fluid renderBlock z-position *this can gradually climb a hill and still sort correctly, 
	// instead of visually snapping to a layer using trigger lines,
	// and the worldLayer can be used as a collision filter
	worldLayer = game.GetMap().TileMap().LayerFromZPosition(eMath::NearestInt(renderImage->RenderBlock()[0].z));
}
