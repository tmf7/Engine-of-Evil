#include "Entity.h"
#include "Game.h"

//***************
// eEntity::Spawn
// TODO: set the initial frame of animation
// TODO: determine a failure to spawn condition (ie when to return false)
// TODO: call a GetSpawnPoint() to use a list of (semi-)pre-defined spawn points,
// possibly parsed from a file
//***************
bool eEntity::Spawn() {
	eImage * spriteImage = NULL;

	spriteImage = game.GetImageManager().GetImage("graphics/hero.bmp", nullptr);
	if (spriteImage == nullptr)
		return false;

	sprite.SetImage(*spriteImage);
	localBounds.ExpandSelf(8);	// 16 x 16 square with (0, 0) at its center
	SetOrigin(eVec2(8.0f, 8.0f));
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

