#ifndef EVIL_BUTTON_H
#define EVIL_BUTTON_H

#include "GameObject.h"

//******************************
//		eButton
// interactive animated graphic
// that hold three states: 
// pressed, mouse over, and triggered
// TODO: add a eButtonManager that can update all buttons in one call
// TODO: user defines IsTriggered() behavior 
// TODO: user sets the button image's frame (to allow for extension to radio buttons, or unanimated buttons)
//******************************
class eButton : public eGameObject {
public:

	void				Init(const eVec2 & canvasPosition, const eVec2 & size);
	bool				IsTriggered() const;
	bool				IsPressed() const;
	bool				IsMouseOver() const;
	void				ClearTrigger();
	void				Think();
	void				Draw();

	void				SetTriggeredCallback(callbackFn_t callback);

private:

	eBounds				clickRegion;

	// TODO: give eCanvas a list of UI elements to draw in order (no sorting) 
	// (eg: buttons, plain images, text boxes[?]...all of which may need scaling...eUIElement class w/scale and...offset w/in canvas...and if collision tests occur against their rects)
	// TODO: UI_Image and eAnimationController compose the eButton, while eButton itself is focused only on the TriggeredCallback....but also setting animcontroller trigger values

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