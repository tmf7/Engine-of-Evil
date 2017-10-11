#include "Game.h"

typedef struct SpawnArgs_s {
	std::string spriteFilename;
	int spriteLayer;			// TODO: this must be modifiable
//	int spriteOrderInLayer;
	eVec2 origin;
	eVec2 facing;
	float speed;
//	int colliderType;		// make a class enum... or all AABB/OBB/circle...
	eVec2 colliderWidthHeight;
	eVec2 colliderOffset;
	int collisionMask;		// enum different default masks, but allow user-defined masks
} SpawnArgs_t;

//***************
// eEntity::Spawn
// TODO: spawnArgs: position, facing angle/unit-velocity, speed
// TODO: spawnArgs: collider type, collider dimensions, collider offset, collider mask (separate initialization on its own!)
// TODO: spawnArgs: sprite image file, sprite layer, order in layer, initial animation frame
//***************
bool eEntity::Spawn(/*const char * entityFilename, eVec2 & worldPosition*/) {

	SpawnArgs_t test = { "graphics/hero.png", MAX_LAYER , vec2_zero, vec2_oneZero, 10.0f, eVec2(10.0f, 10.0f), vec2_zero, 2 };
	test.spriteFilename;	

	std::shared_ptr<eImage> spriteImage;
	if (!game.GetImageManager().LoadImage("graphics/hero.png", SDL_TEXTUREACCESS_STATIC, spriteImage))
		return false;

	// TODO: call sprite.Init("filename.sprite");
	// where all the eAnimations have ALREADY been eAnimationManager::BatchLoad(filename)
	// and the sprite just eAnimationManager.GetAnimation a bunch of times according to the sprite's file
	// AND potentially loads various control varibles from the sprite's file as well (maybe)
	// TODO: after the sprite has been fully loaded WRITE a spriteController class
	// that dictates transitions between eAnimations and intra-animation-sequences (similar to Button.h)
	sprite.SetImage(spriteImage);		// TODO: change this to a sprite.Init(...) maybe and return false if it fails

	collisionModel.SetActive(true);
	collisionModel.LocalBounds().ExpandSelf(8);			// FIXME: 16 x 16 square with (0, 0) at its center,
	collisionModel.SetOrigin(eVec2(192.0f, 192.0f));
	collisionModel.Velocity() = vec2_zero;

	imageColliderOffset = eVec2(-32.0f, -45.0f);//eVec2(-58.0f, -90.0f);	// FIXME: import from an entity definition file

	UpdateRenderImageOrigin();
	UpdateRenderImageDisplay();
	return true;
}

//***************
// eEntity::Draw
//***************
void eEntity::Draw() {

	// FIXME: possibly move these two calls elsewhere
	UpdateRenderImageOrigin();
	UpdateRenderImageDisplay();

	auto & cameraBounds = game.GetCamera().CollisionModel().AbsBounds();
	renderImage.worldClip = eBounds(renderImage.origin, renderImage.origin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h));

	if (eCollision::AABBAABBTest(cameraBounds, renderImage.worldClip))
		game.GetRenderer().AddToRenderPool(&renderImage, RENDERTYPE_DYNAMIC, true);
}

//*************
// eEntity::UpdateRenderImageOrigin
// backend collision occurs on a 2D top-down grid
// UpdateRenderImageOrigin ensures only the visuals are isometric
//*************
void eEntity::UpdateRenderImageOrigin() {
	renderImage.origin = collisionModel.AbsBounds()[0];			// FIXME(?): eTile::renderImage::origin is unmoving in iso. world space (regardless of collision)
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
	renderImage.image = sprite.GetImage();
	renderImage.srcRect = &sprite.GetFrameHack();
	renderImage.SetLayer(1);		// DEBUG: test starting layer

// FREEHILL BEGIN 3d quicksort test
	static float baseDepth = 64.0f;
	float increment = 2.0f;

	auto & input = game.GetInput();
	if (input.KeyPressed(SDL_SCANCODE_H))
		baseDepth += increment;
	else if (input.KeyPressed(SDL_SCANCODE_L))
		baseDepth -= increment;

	eVec2 originHack = collisionModel.AbsBounds()[0];
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

