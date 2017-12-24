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
#ifndef EVIL_INPUT_H
#define EVIL_INPUT_H

#include "Definitions.h"
#include "Class.h"

//*************************************
//				eInput
// handles user input from mouse and keyboard
//*************************************
class eInput : public eClass {
public:

	friend class eGame;

public:

						eInput();
						eInput(const eInput & other) = delete;
						eInput(eInput && other) = delete;
						~eInput();

	eInput &			operator=(const eInput & other) = delete;
	eInput				operator=(eInput && other) = delete;

	void				Init();
	void				Update();

	int					KeyHeld(int key) const;
	int					KeyPressed(int key) const;
	int					KeyReleased(int key) const;

	int					MouseHeld(int button) const;
	int					MousePressed(int button) const;
	int					MouseReleased(int button) const;
	bool				MouseMoved() const;
	int					GetMouseX() const;
	int					GetMouseY() const;
	int					GetMouseScroll() const;
	void				HideCursor(bool hide = true) const;

	virtual int			GetClassType() const override				{ return CLASS_INPUT; }
	virtual bool		IsClassType(int classType) const override	{ 
							if(classType == CLASS_INPUT) 
								return true; 
							return eClass::IsClassType(classType); 
						}

private:

	void				PollEvents();
	void				SetMouseWheelState(int wheelDirection);

private:

	Uint8 *				keys;
	Uint8 *				prevKeys;
	int					numKeys;

	int					mouseButtons[3];
	int					prevMouseButtons[3];
	int					mouseX;
	int					mouseY;
	int					mouseScroll;
	int					oldMouseX;
	int					oldMouseY;
};

//***************
// eInput::eInput
//***************
inline eInput::eInput() 
	: keys(nullptr), prevKeys(nullptr) {
}

#endif /* EVIL_INPUT_H */

