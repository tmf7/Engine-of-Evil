#include "Button.h"
#include "Game.h"

//*************
// eButton::Init
// TODO: parse buttonDefFile
// read rectangles, and default state
//**************
void eButton::Init(const eVec2 & position, const eVec2 & size) {
	clickRegion = eBounds(position, position + size);
	mouseOver = false;
	pressed = false;
	triggered = false;
}


//*************
// eButton::Think
// TODO: some object will instantiate an eButton, which means it can have clear access to its state,
// which means during its own "Update" it can check if its buttons have fired and execute relevant fn calls
// EG: MainMenuState calls eCanvas::AddButton("Start Game"), but retains a reference to that button for its own use
// TODO(~): callback fn to allow the canvas to be used across multiple contexts...unless the other context could do a eCanvas::GetButton("Start Menu"), and check if it's non-nullptr
// OR: just have a vector of void (*triggeredCallback)(void) fns that all get called upon triggering
// PROBLEM: callback that needs a LoadLevel(levelNum_or_levelName)
// SOLUTION: register an observer (eGameObject *) to the eButton (eGameObject)
// ALONG with a function and up to one argument...nope...
//**************
void eButton::Think() {
	auto & input = game->GetInput();
	eVec2 screenPoint = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());

	if (eCollision::AABBContainsPoint(clickRegion, screenPoint)) {
		mouseOver = true;

		if (!pressed && (input.MousePressed(SDL_BUTTON_LEFT) || input.MousePressed(SDL_BUTTON_RIGHT) || input.MousePressed(SDL_BUTTON_MIDDLE))) {
			pressed = true;
		} else if (pressed && (input.MouseReleased(SDL_BUTTON_LEFT) || input.MouseReleased(SDL_BUTTON_RIGHT) || input.MouseReleased(SDL_BUTTON_MIDDLE))) {
			triggered = true;
			pressed = false;
		}

	} else { 
		mouseOver = false; 
	}
}

//*************
// eButton::Draw
// TODO: use animationController to allow the user to set the AnimationState (and renderImage)
// TODO: initialize the button to draw on its associated eCanvas (a UI eGameObject with its own eRenderTarget
// that gets positioned and blitted to either a camera or the main renderTarget depending on its configuration)
// IE: draw on the eCanvas
//**************
void eButton::Draw() {
	game->GetRenderer().AddToOverlayRenderPool(renderImage.get());
}