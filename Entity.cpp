#include "Game.h"

//***************
// eEntity::Spawn
// TODO: generalize this to account for eCamera initilization (speed, size, etc)
//***************
bool eEntity::Spawn() {

	// TODO: pass in some spawn arguments for the animation definitions file
	// TODO: set the initial (no input/movement) default animation
	std::shared_ptr<eImage> spriteImage;
	if (!game.GetImageManager().LoadImage("graphics/hero.bmp", SDL_TEXTUREACCESS_STATIC, spriteImage))
		return false;

	// TODO: call sprite.Init("imageTilerBatchLoadFilename.sprite");
	// where all the imageTilers have ALREADY been eImageTilerManager::BatchLoad(filename)
	// and the sprite just eImageTilerManager.GetTiler a bunch of times according to the sprite's file
	// AND potentially loads various control varibles from the sprite's file as well (maybe)
	// TODO: after the sprite has been fully loaded WRITE a spriteController class
	// that dictates transitions between tilers and intra-tiler-sequences (similar to Button.h)
	sprite.SetImage(spriteImage);		// TODO: change this to a sprite.Init(...) maybe and return false if it fails

	collisionModel.LocalBounds().ExpandSelf(8);			// FIXME: 16 x 16 square with (0, 0) at its center, 
	collisionModel.SetOrigin(eVec2(8.0f, 8.0f));		// TODO: call a GetSpawnPoint() to use a list of procedurally/file-defined spawn points
	imageOffset = - collisionModel.Origin();			// DEBUG: test offset
	UpdateRenderImageOrigin();
	UpdateRenderImageDisplay();
	return true;
}

//***************
// eEntity::Draw
//***************
void eEntity::Draw() {
	auto & cameraBounds = game.GetCamera().CollisionModel().AbsBounds();
	eBounds dstBounds = eBounds(renderImage.origin, renderImage.origin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h));

	if (eCollision::AABBAABBTest(cameraBounds, dstBounds)) {	
		eMath::CartesianToIsometric(dstBounds[0].x, dstBounds[0].y);
		dstBounds.TranslateSelf(-game.GetCamera().CollisionModel().AbsBounds()[0]);
		dstBounds[0].SnapInt();
//		dstBounds[1].SnapInt();		// DEBUG: unnecessary given renderImage.srcRect dimensions

		renderImage.dstRect = { (int)dstBounds[0].x, (int)dstBounds[0].y, renderImage.srcRect->w, renderImage.srcRect->h };
		game.GetRenderer().AddToRenderPool(&renderImage, RENDERTYPE_DYNAMIC);
	}
}

//*************
// eEntity::UpdateRenderImageOrigin
//*************
void eEntity::UpdateRenderImageOrigin() {
	renderImage.origin = collisionModel.Origin() + imageOffset;
}

//*************
// eEntity::UpdateRenderImageDisplay
// TODO: uses the eSprite to set the animation image and frame data
//*************
void eEntity::UpdateRenderImageDisplay() {
	renderImage.image = sprite.GetImage();
	renderImage.srcRect = &sprite.GetFrameHack();
	renderImage.SetLayer(1);		// DEBUG: test layer == 1
}

//*************
// eEntity::GetRenderImage
//*************
renderImage_t * eEntity::GetRenderImage() {
	return &renderImage;
}

