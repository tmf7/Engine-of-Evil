#include "Game.h"

//**************
// eEntity::eEntity
// FIXME(?): what if other.collisionModel (etc) are nullptr? exception thrown?
//**************
eEntity::eEntity(const eEntity & other) {
	imageColliderOffset = other.imageColliderOffset;
	renderImage			= other.renderImage;							// FIXME: theres std::shared_ptr<eImage> in here, may be a move/copy problem
	sprite				= other.sprite ? std::make_shared<eSprite>(*other.sprite) : nullptr;
	collisionModel		= other.collisionModel ? std::make_shared<eCollisionModel>(*other.collisionModel) : nullptr;
	movementPlanner		= other.movementPlanner ? std::make_shared<eMovement>(*other.movementPlanner) : nullptr;
	prefabFilename		= other.prefabFilename;
	prefabManagerIndex	= other.prefabManagerIndex;
	spawnedEntityID		= other.spawnedEntityID;
}

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(eEntity && other)
	: imageColliderOffset(std::move(other.imageColliderOffset)),
	  renderImage(std::move(other.renderImage)),					// FIXME: theres std::shared_ptr<eImage> in here, may be a move/copy problem
	  sprite(std::move(other.sprite)),
	  collisionModel(std::move(other.collisionModel)),
	  movementPlanner(std::move(other.movementPlanner)),
	  prefabFilename(std::move(other.prefabFilename)),
	  prefabManagerIndex(other.prefabManagerIndex),
	  spawnedEntityID(other.spawnedEntityID) {

}

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(const entitySpawnArgs_t & spawnArgs)
	: prefabFilename(spawnArgs.prefabFilename),
	  imageColliderOffset(spawnArgs.imageColliderOffset),
	  prefabManagerIndex(spawnArgs.prefabManagerIndex),
	  spawnedEntityID(-1) {

	if (spawnArgs.movementSpeed) {
		// FIXME/BUG(!): *this may move if its container re-allocates, or is otherwise moved
		movementPlanner = std::make_shared<eMovement>(this, spawnArgs.movementSpeed);
	}

	if (!spawnArgs.localBounds.IsEmpty()) {
		collisionModel->LocalBounds() = spawnArgs.localBounds;
		collisionModel->SetActive(spawnArgs.collisionActive);
	}

	// init sprite and renderBlock for draw order sorting
	if (!spawnArgs.spriteFilename.empty()) {
		sprite = std::make_shared<eSprite>();	// TODO: sprite initialization should be just this one line
		std::shared_ptr<eImage> spriteImage;
		if (!game.GetImageManager().LoadImage(spawnArgs.spriteFilename.c_str(), SDL_TEXTUREACCESS_STATIC, spriteImage))
			throw badEntityCtorException(spawnArgs.spriteFilename.c_str());

		sprite->SetImage(spriteImage);			
		auto & localMins = spawnArgs.localBounds[0];
		eVec3 blockMins = eVec3(localMins.x, localMins.y, 0.0f);
		renderImage.renderBlock = eBounds3D(blockMins, blockMins + spawnArgs.renderBlockSize);
	}
}

//**************
// eEntity::operator=
//**************
eEntity & eEntity::operator=(eEntity other) {
	std::swap(imageColliderOffset, other.imageColliderOffset);
	std::swap(renderImage, other.renderImage);
	std::swap(sprite, other.sprite);
	std::swap(collisionModel, other.collisionModel);
	std::swap(movementPlanner, other.movementPlanner);
	std::swap(prefabFilename, other.prefabFilename);
	std::swap(prefabManagerIndex, other.prefabManagerIndex);
	std::swap(spawnedEntityID, other.spawnedEntityID);
    return *this;
}

//***************
// eEntity::Spawn
// copy a prefab eEntity and add unique details
// TODO: position via a single stack eTransform, not the eCollisionModel, or renderImage_t, or eSprite
//***************
bool eEntity::Spawn(const int entityPrefabIndex, const eVec3 & worldPosition /*, const eVec2 & facingDir*/) {

	// TODO: allocate space for an entity (std::make_shared<eEntity>(spawnArgs))
	// spawn it
	// upon failure, catch the thrown bad exception
	std::shared_ptr<eEntity> prefabEntity = nullptr;
	if (!game.GetEntityPrefabManager().GetPrefab(entityPrefabIndex, prefabEntity));
		return false;

	int spawnID = game.NumEntities();
	try {
		// FIXME/BUG(!): new allocation and control block, but not a deep copy of eEntity shared pointers
		game.AddEntity(std::make_shared<eEntity>(prefabEntity));
		auto & newEntity = game.GetEntity(spawnID);
		newEntity->spawnedEntityID = spawnID;

		// FIXME: use worldPosition.z to determine a layer (ie: set a const layer depth, or depth for each layer)
		newEntity->renderImage.renderBlock += worldPosition;
		newEntity->collisionModel->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	//	if (newEntity->spriteController != nullptr)	// TODO: eMovement may be opposite facing, and eEntity may not have a eSprite, so only eSpriteController cares about facing
	//		newEntity->spriteController->SetFacingDirection(facingDir);		

		newEntity->UpdateRenderImageOrigin();
		newEntity->UpdateRenderImageDisplay();
		return true;
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (" + std::to_string(spawnID) + ") spawn failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
}

//***************
// eEntity::Draw
//***************
void eEntity::Draw() {
	// FIXME(?): move these two calls elsewhere
	UpdateRenderImageOrigin();
	UpdateRenderImageDisplay();

	auto & cameraBounds = game.GetCamera().CollisionModel().AbsBounds();
	renderImage.worldClip = eBounds(renderImage.origin, renderImage.origin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h));

	if (eCollision::AABBAABBTest(cameraBounds, renderImage.worldClip))
		game.GetRenderer().AddToRenderPool(&renderImage, RENDERTYPE_DYNAMIC, true);
}

//***************
// eEntity::Think
//***************
void eEntity::Think() {
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw() {
}

//*************
// eEntity::UpdateRenderImageOrigin
// backend collision occurs on a 2D top-down grid
// UpdateRenderImageOrigin ensures only the visuals are isometric
//*************
void eEntity::UpdateRenderImageOrigin() {
	renderImage.origin = collisionModel->AbsBounds()[0];		// FIXME(?): eTile::renderImage::origin is unmoving in iso. world space (regardless of collision)
																// SOLUTION: treat all renderImage-collisionModel relations the same
																// everthing can have a Transform...position, orientation, scale
																// then collisionModels have origins at their center w/offset from the transform
																// and renderImage_ts have origins at their top-left corner w/ offset from the transform
	eMath::CartesianToIsometric(renderImage.origin.x, renderImage.origin.y);
	renderImage.origin += imageColliderOffset;
}

//*************
// eEntity::UpdateRenderImageDisplay
// TODO: uses the eSprite to set the animation image and frame data
// TODO: move this to eEntity::Think, and just call sprite.Update(renderImage);
//*************
void eEntity::UpdateRenderImageDisplay() {
	renderImage.image = sprite->GetImage();
	renderImage.srcRect = &sprite->GetFrameHack();
	renderImage.SetLayer(1);		// DEBUG: test starting layer

// FREEHILL BEGIN 3d quicksort test
	static float baseDepth = 64.0f;
	float increment = 2.0f;

	auto & input = game.GetInput();
	if (input.KeyPressed(SDL_SCANCODE_H))
		baseDepth += increment;
	else if (input.KeyPressed(SDL_SCANCODE_L))
		baseDepth -= increment;

	eVec2 originHack = collisionModel->AbsBounds()[0];
	eVec2 collisionOffsetHack = vec2_zero;
	float baseDepthHack = 0.0f;
	switch(renderImage.layer) {
		case 0: break;
		case 1: baseDepthHack = 1.0f; break;
		case 2: baseDepthHack = baseDepth; break;
	}
	renderImage.renderBlock = eBounds3D(eVec3(originHack.x, originHack.y, 0.0f));	// FIXME: re-position and resize (find a way to avoid resizing later)
	renderImage.renderBlock[1] = renderImage.renderBlock[0] + eVec3(16.0f, 16.0f, (float)renderImage.srcRect->h);	// FIXME: zSize here may vary depending on entity (like grass tiles have 0 zSize)
	renderImage.renderBlock += eVec3(collisionOffsetHack.x, collisionOffsetHack.y, baseDepthHack);

// FREEHILL END 3d quicksort test
}

//*************
// eEntity::GetRenderImage
//*************
renderImage_t * eEntity::GetRenderImage() {
	return &renderImage;
}

