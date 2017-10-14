#include "Game.h"

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(const eEntity & other) {
	imageColliderOffset = other.imageColliderOffset;
	renderImage			= other.renderImage;		// DEBUG: shallow std::shared_ptr<eImage> assignment, and std::vector<renderImage_s *> deep copy ctor
	sprite				= other.sprite ? std::make_shared<eSprite>(*other.sprite) : nullptr;
	collisionModel		= other.collisionModel ? std::make_shared<eCollisionModel>(*other.collisionModel) : nullptr;
	movementPlanner		= other.movementPlanner ? std::make_shared<eMovement>(*other.movementPlanner) : nullptr;		// FIXME/BUG(!): bad deep copy ???
	prefabFilename		= other.prefabFilename;
	prefabManagerIndex	= other.prefabManagerIndex;
	spawnedEntityID		= other.spawnedEntityID;
}

//**************
// eEntity::eEntity
//**************
eEntity::eEntity(eEntity && other)
	: imageColliderOffset(std::move(other.imageColliderOffset)),
	  renderImage(std::move(other.renderImage)),	// DEBUG: shallow std::shared_ptr<eImage> swap, and std::vector<renderImage_s *> move ctor
	  sprite(std::move(other.sprite)),
	  collisionModel(std::move(other.collisionModel)),
	  movementPlanner(std::move(other.movementPlanner)),		// FIXME/BUG(!): bad deep move ctor ???
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

	if (!spawnArgs.localBounds.IsEmpty()) {
		collisionModel = std::make_shared<eCollisionModel>();
		collisionModel->LocalBounds() = spawnArgs.localBounds;
		collisionModel->SetActive(spawnArgs.collisionActive);
	
		if (spawnArgs.movementSpeed) {
			// FIXME/BUG(!): *this may move if its container re-allocates, or is otherwise moved
			// SOLUTION: the movementPlanner.owner ptr value must update if *this moves,
			// or...???
			movementPlanner = std::make_shared<eMovement>(this, spawnArgs.movementSpeed);
		}
	}

	// init sprite and renderBlock for draw order sorting
	if (!spawnArgs.spriteFilename.empty()) {
		sprite = std::make_shared<eSprite>();	// TODO: sprite initialization should be just this one line
		std::shared_ptr<eImage> spriteImage = nullptr;
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
	std::swap(renderImage, other.renderImage);		// DEBUG: shallow std::shared_ptr<eImage> swap, and std::vector<renderImage_s *> move ctor
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
	std::shared_ptr<eEntity> prefabEntity = nullptr;
	if (!game.GetEntityPrefabManager().GetPrefab(entityPrefabIndex, prefabEntity))
		return false;

	int spawnID = game.NumEntities();
	try {
		// FIXME/BUG(!): ensure this invokes the copy ctor and not the move ctor
		// by checking the contents of game.entityPrefabManager.prefabList
		// and the contents of game.entities before & after this line
		game.AddEntity(std::make_shared<eEntity>(*prefabEntity));
		auto & newEntity = game.GetEntity(spawnID);
		newEntity->spawnedEntityID = spawnID;

		// TODO: use worldPosition.z to determine a layer (ie: set a const layer depth, or depth for each layer in eSpatialIndexGrid)
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
	renderImage.origin = collisionModel->AbsBounds()[0];		// FIXME(later): eTile::renderImage::origin is unmoving in iso. world space (regardless of collision)
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
	float zChange = 0.0f;
	auto & input = game.GetInput();
	if (input.KeyPressed(SDL_SCANCODE_H))
		zChange = 2.0f;
	else if (input.KeyPressed(SDL_SCANCODE_L))
		zChange = -2.0f;
	
	// translate from old renderBlock pos, to new renderBlock pos
	// DEBUG: renderBlock and collisionModel currently designed to align, while offsetting renderImage.origin instead
	eVec2 collisionMins = collisionModel->AbsBounds()[0];
	eVec3 renderBlockMins = renderImage.renderBlock[0];
	renderImage.renderBlock += eVec3(renderBlockMins.x - collisionMins.x, renderBlockMins.y - collisionMins.y, zChange);
// FREEHILL END 3d quicksort test
}

//*************
// eEntity::GetRenderImage
//*************
renderImage_t * eEntity::GetRenderImage() {
	return &renderImage;
}

