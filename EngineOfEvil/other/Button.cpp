#include "Button.h"
#include "Editor.h"

//*************
// eButton::Init
// TODO: parse buttonDefFile
// read rectangles, and default state
//**************
void eButton::Init(const SDL_Rect & screenRegion, const std::shared_ptr<eImageTiler> & tiler) {
	states = tiler;
	clickRegion = screenRegion;
	mouseOver = false;
	pressed = false;
	triggered = false;
}

//*************
// eButton::Think
//**************
void eButton::Think() {
	eInput & input = editor.GetInput();
	SDL_Point testPoint{ input.GetMouseX(), input.GetMouseY() };

	if (SDL_PointInRect(&testPoint, &clickRegion)) {
		mouseOver = true;
		if (!pressed && input.MousePressed(SDL_BUTTON_LEFT)) {
			pressed = true;
		} else if (pressed && input.MouseReleased(SDL_BUTTON_LEFT)) {
			triggered = true;
			pressed = false;
		}
	} else { 
		mouseOver = false; 
	}
}

//*************
// eButton::Draw
// TODO: modify this logic to allow for buttons with more than 3 stateImages
// IE the old eImageTiler::GetFrame(void) call here doesn't work anymore, but
// somwthing will have to set which frame to use (if not eButton itself)
// perhaps the user sets a callback to an animation controller (that may only have one frame listed)???
//**************
void eButton::Draw() {
	renderImage_t test{ states->Source(), &states->GetFrame(state), clickRegion, MAX_LAYER };
	editor.GetRenderer().AddToRenderPool(test, RENDERTYPE_STATIC);
}