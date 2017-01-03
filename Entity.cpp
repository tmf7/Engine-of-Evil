#include "Entity.h"
#include "Game.h"

//***************
// eEntity::Spawn
//***************
bool eEntity::Spawn() {

	// TODO: pass in some spawn arguments for the animation definitions file
	// TODO: set the initial (no input/movement) default animation
	SDL_Texture * spriteTexture = game.GetTextureManager().GetTexture("graphics/hero.bmp");
	if (spriteTexture == nullptr)
		return false;

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
	game.GetRenderer().AddToRenderQueue(renderImage_t(drawPoint, sprite.Image(), 1));	// DEBUG: test layer == 1
}

