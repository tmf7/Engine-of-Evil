/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_BUTTON_H
#define EVIL_BUTTON_H

#include "GameObject.h"

//******************************
//		eButton
// interactive animated graphic
// that has four operational states: 
// pressed, triggered, mouse-over, and disabled
//******************************
class eButton : public eGameObject {
public:

	void				Init(const eVec2 & canvasPosition, const eVec2 & size);		// TODO: add an eAnimationController and eRenderImageBase so *this can draw to an eCanvas
	bool				IsTriggered() const;
	bool				IsPressed() const;
	bool				IsMouseOver() const;
	void				ClearTrigger();

	virtual int								GetClassType() const override				{ return CLASS_GAMEOBJECT; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_GAMEOBJECT) 
													return true; 
												return eGameObject::IsClassType(classType); 
											}

	virtual void							Init() override;
	virtual void							Think()	override;
	virtual void							DebugDraw(eRenderTarget * renderTarget)	override;

private:

	// FIXME: make these transition parameters in a default eAnimationController
	// TODO: Normal, Highlighted, Pressed, Disabled eAnimationStates (in an .ectrl file)
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