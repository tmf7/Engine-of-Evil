#include "GameObject.h"
#include "Game.h"

//**************
// eGameObject::eGameObject
//**************
eGameObject::eGameObject(const eGameObject & other) 
	: orthoOrigin(other.orthoOrigin),
	  worldLayer(other.worldLayer),		
	  oldWorldLayer(other.oldWorldLayer),
	  isStatic(other.isStatic),
	  zPosition(other.zPosition) {

	// DEBUG: using std::make_unique and release to prevent leaks in the event an allocation fails
	if (other.renderImage != nullptr)			renderImage.reset(static_cast<eRenderImage *>(other.renderImage->GetCopy().release()));
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
	  oldWorldLayer(other.oldWorldLayer),
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
	std::swap(oldWorldLayer, other.oldWorldLayer);
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

//*************
// eGameObject::AddRenderImage
// returns true if an eRenderImage has been added to *this
// returns false if not because the filename is invalid, or the file is unreadable
//*************
bool eGameObject::AddRenderImage( const std::string & spriteFilename, const eVec3 & renderBlockSize, int initialSpriteFrame, const eVec2 & renderImageOffset, bool isPlayerSelectable ) {
	if (spriteFilename.empty())
		return false;

	renderImage = std::make_unique<eRenderImage>(this);
	std::shared_ptr<eImage> spriteImage = nullptr;
	if (!game.GetImageManager().LoadAndGet(spriteFilename.c_str(), spriteImage))
		return false;

	renderImage->SetImage(spriteImage->GetManagerIndex());
	if (initialSpriteFrame < 0 || initialSpriteFrame > spriteImage->NumSubframes())
		initialSpriteFrame = 0;

	renderImage->SetImageFrame(initialSpriteFrame);
	renderImage->SetOffset(renderImageOffset);
	renderImage->SetRenderBlockSize(renderBlockSize);
	renderImage->SetIsSelectable(isPlayerSelectable);
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

	collisionModel = std::make_unique<eCollisionModel>(this);
	collisionModel->SetLocalBounds(localBounds);
	collisionModel->SetOffset(colliderOffset);
	collisionModel->SetActive(collisionActive);
	return true;
}

//*************
// eGameObject::AddAnimationController
// returns true if an eAnimationController has been added to *this
// returns false if not because the filename is invalid, the file is unreadable, 
// or there is no eRenderImage attached to *this
// DEBUG: call AddRenderImage before calling this, because it depends on an eRenderImage object
//*************
bool eGameObject::AddAnimationController( const std::string & animationControllerFilename ) {
	if ( renderImage == nullptr || animationControllerFilename.empty() )
		return false;

	std::shared_ptr<eAnimationController> prefabAnimationController = nullptr;
	if ( !game.GetAnimationControllerManager().LoadAndGet( animationControllerFilename.c_str(), prefabAnimationController ) )
		return false;

	animationController = std::make_unique< eAnimationController >( *prefabAnimationController );
	animationController->SetOwner( this );
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