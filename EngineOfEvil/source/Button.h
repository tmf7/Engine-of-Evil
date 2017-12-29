#ifndef EVIL_BUTTON_H
#define EVIL_BUTTON_H

#include "UIElement.h"

//******************************
//		eButton
// interactive animated graphic
// that hold three states: 
// pressed, mouse over, and triggered
// TODO: add a eButtonManager that can update all buttons in one call
// TODO: user defines IsTriggered() behavior 
// TODO: user sets the button image's frame (to allow for extension to radio buttons, or unanimated buttons)
//******************************
class eButton : public eUIElement {
public:

	void				Init(const eVec2 & canvasPosition, const eVec2 & size);
	bool				IsTriggered() const;
	bool				IsPressed() const;
	bool				IsMouseOver() const;
	void				ClearTrigger();

	void				Think();
	void				Draw();

private:

	// eRenderImageBase					image;				// simplified version of eRenderImageIsometric for use on an eCanvas instead of with an eGameObject
	eAnimationController				animController;		// TODO: modify this to have POINTERs to eImage and ImageFrame data (instead of indirect to renderImage through owner) (ie: SetTarget(eImage *, SDL_Rect *))


	// TODO: eUIImage and eAnimationController compose the eButton, while eButton itself is focused only on animcontroller trigger values, and having its state queried

	// FIXME: make these transition parameters in a default eAnimationController
	// TODO: Normal, Highlighted, Pressed, Disabled eAnimationStates (in a file)
	bool				mouseOver;
	bool				pressed;
	bool				triggered;
	bool				disabled;
};

//*************
// eButton::IsPressed
//**************
inline bool eButton::IsPressed() const {
	return pressed;
}

//*************
// eButton::IsTriggered
//**************
inline bool eButton::IsTriggered() const {
	return triggered;
}

//*************
// eButton::IsMouseOver
//**************
inline bool eButton::IsMouseOver() const {
	return mouseOver;
}

//*************
// eButton::ClearTrigger
//**************
inline void eButton::ClearTrigger() {
	triggered = false;
}

#endif /* EVIL_BUTTON_H */