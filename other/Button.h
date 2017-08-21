#ifndef EVIL_BUTTON_H
#define EVIL_BUTTON_H

#include "ImageTiler.h"

//******************************
//		eButton
// interactive animated graphic
// that hold three states: 
// pressed, mouse over, and triggered
// TODO: add a eButtonManager that can update all buttons in one call
// TODO: user defines IsTriggered() behavior 
// TODO: user sets the button image's frame (to allow for extension to radio buttons, or unanimated buttons)
//******************************
class eButton {
public:

	void				Init(const SDL_Rect & screenRegion, const std::shared_ptr<eImageTiler> & tiler);
	bool				IsTriggered() const;
	bool				IsPressed() const;
	bool				IsMouseOver() const;
	void				ClearTrigger();
	void				Think();
	void				Draw();
	void				SetStateEnum(const Uint8 state);
	Uint8				GetStateEnum() const;

private:

	std::shared_ptr<eImageTiler>	states;	// overall image with sub-frame access

	SDL_Rect			clickRegion;
	Uint8				state;
	bool				mouseOver;
	bool				pressed;
	bool				triggered;
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

//*************
// eButton::SetStateEnum
//**************
inline void eButton::SetStateEnum(const Uint8 state) {
	this->state = state;
}

//*************
// eButton::GetStateEnum
//**************
inline Uint8 eButton::GetStateEnum() const {
	return state;
}

#endif /* EVIL_BUTTON_H */