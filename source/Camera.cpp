#include "Game.h"

//***************
// eCamera::eCamera
//***************
eCamera::eCamera()
	: camSpeed(defaultCamSpeed) {
	  collisionModel.SetOwner(this);
	  collisionModel.SetActive(false);	// DEBUG: does not participate in normal collision detection (ie UpdateAreas doesn't happen)
}

//***************
// eCamera::Init
// starting view of map
// TODO: allow the localBounds to be resized as the window resizes or goes fullscreen
//***************
void eCamera::Init() {
	SetZoom(1);
	collisionModel.SetOrigin(collisionModel.LocalBounds()[1] / 2.0f);
}

//***************
// eCamera::Think
// FIXME/TODO: modify the movement limits (either stay inside the "diamond" or allow some minimal wander beyond it
// EG: an overall larger rectangle that the diamond is within [risky, given that some corner gaps would lead to total abyss])
//***************
void eCamera::Think() {
	eInput * input;
	eVec2 correction;
	float x, y;
/*
	static const int maxX = game.GetMap().TileMap().Width() > (int)collisionModel.LocalBounds().Width() 
							? game.GetMap().TileMap().Width() 
							: (int)collisionModel.LocalBounds().Width();

	static const int maxY = game.GetMap().TileMap().Height() > (int)collisionModel.LocalBounds().Height() 
							? game.GetMap().TileMap().Height() 
							: (int)collisionModel.LocalBounds().Height();
*/
	input = &game.GetInput();

	// TODO/FIXME (much later): zoom in/out adversly affects drawing coordinates
	// and number of tiles drawn to the screen (which is currently more-or-less hardcoded)
	// So right now this is more of an experiment/novelty than a feature
	float oldZoomLevel = zoomLevel;
	eVec2 oldOrigin = collisionModel.Origin();
	if (input->KeyPressed(SDL_SCANCODE_EQUALS))
		SetZoom(zoomLevel + zoomIncrement);
	else if (input->KeyPressed(SDL_SCANCODE_MINUS))
		SetZoom(zoomLevel - zoomIncrement);

	if (input->KeyHeld(SDL_SCANCODE_SPACE)) {
		eVec2 snapFocus = game.GetEntity(0)->CollisionModel().Origin();		// FIXME: 0th eEntity should not be the default thing to snap focus to
		eMath::CartesianToIsometric(snapFocus.x, snapFocus.y);
		collisionModel.SetOrigin(snapFocus);
	} else {
		x = camSpeed * (float)(input->KeyHeld(SDL_SCANCODE_D) - input->KeyHeld(SDL_SCANCODE_A));
		y = camSpeed * (float)(input->KeyHeld(SDL_SCANCODE_S) - input->KeyHeld(SDL_SCANCODE_W));
		collisionModel.Velocity().Set(x, y);
		if (collisionModel.Velocity() != vec2_zero)
			collisionModel.UpdateOrigin();
	}

	moved = (zoomLevel != oldZoomLevel || collisionModel.Origin() != oldOrigin);

/*
	// collision response with map edge
	// TODO(?): move this to a collision detection/handling class
	correction = vec2_zero;
	if (absBounds[0].x < 0)
		correction.x = -absBounds[0].x;
	else if (absBounds[1].x > maxX)
		correction.x = maxX - absBounds[1].x;

	if (absBounds[0].y < 0)
		correction.y = -absBounds[0].y;
	else if (absBounds[1].y > maxY)
		correction.y = maxY - absBounds[1].y;

	if (correction != vec2_zero)
		SetOrigin(origin + correction);
*/
}

//***************
// eCamera::SetZoom
//***************
void eCamera::SetZoom(float level) {
	if (level < minZoom)
		level = minZoom;
	else if (level > maxZoom)
		level = maxZoom;

	zoomLevel = level;

	eVec2 screenBottomRight = eVec2((float)game.GetRenderer().ViewArea().w, (float)game.GetRenderer().ViewArea().h);
	screenBottomRight *= level;

	// variable rectangle with (0, 0) at its center)
	collisionModel.LocalBounds() = eBounds(-screenBottomRight / 2.0f, screenBottomRight / 2.0f);
}

//**************
// eCamera::ScreenToWorldPosition
// returns current position of screenPoint over the 2D orthographic game world
//**************
eVec2 eCamera::ScreenToWorldPosition(const eVec2 & screenPoint) const {
	eVec2 worldPoint = screenPoint + collisionModel.AbsBounds()[0];
	eMath::IsometricToCartesian(worldPoint.x, worldPoint.y);
	return worldPoint;
}

//**************
// eCamera::MouseWorldPosition
// returns current position of mouse over the isometric game world
//**************
eVec2 eCamera::MouseWorldPosition() const {
	auto & input = game.GetInput();
	eVec2 screenPoint = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());
	return ScreenToWorldPosition(screenPoint);
}

