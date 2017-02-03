#include "Entity.h"
#include "Game.h"

//***************
// eEntity::Spawn
//***************
bool eEntity::Spawn() {

	// TODO: pass in some spawn arguments for the animation definitions file
	// TODO: set the initial (no input/movement) default animation
	std::shared_ptr<eImage> spriteImage;
	if (!game.GetImageManager().GetImage("graphics/hero.bmp", spriteImage));
		return false;

	// TODO: call. sprite.Init("imageTilerBatchLoadFilename.sprite");
	// where all the imageTilers have ALREADY been eImageTilerManager::BatchLoad(filename)
	// and the sprite just eImageTilerManager.GetTiler a bunch of times according to the sprite's file
	// AND potentially loads various control varibles from the sprite's file as well (maybe)
	// TODO: after the sprite has been fully loaded WRITE a spriteController class
	// that dictates transitions between tilers and intra-tiler-sequences (similar to Button.h)
	sprite.SetImage(spriteImage);		// TODO: change this to a sprite.Init(...) maybe and return false if it fails

	localBounds.ExpandSelf(8);			// FIXME: 16 x 16 square with (0, 0) at its center, 
										// this is the current collision box
										// but its also used to position the sprite, move away from this methodology
										// becuase a hit box may be smaller than the sprite... give the sprite an origin?
	SetOrigin(eVec2(8.0f, 8.0f));		// TODO: call a GetSpawnPoint() to use a list of procedurally-defined spawn points
	return true;
}

//***************
// eEntity::Draw
// draw the sprite at its current animation frame
//***************
void eEntity::Draw() {
	eVec2 drawPoint = absBounds[0];
	eMath::CartesianToIsometric(drawPoint.x, drawPoint.y);
	drawPoint -= game.GetCamera().absBounds[0];
	drawPoint.SnapInt();
	game.GetRenderer().AddToRenderPool(renderImage_t(drawPoint, sprite.Image(), 1), RENDERTYPE_DYNAMIC);	// DEBUG: test layer == 1
	// FIXME: like with the Map.cpp draw, this should be shard_ptr<eImage>, srcRect (nullptr if entire image), dstRect (screen postion), layer
}

