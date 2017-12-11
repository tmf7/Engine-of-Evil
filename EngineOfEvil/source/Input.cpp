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
#include "Input.h"

//***************
// eInput::Init
//***************
void eInput::Init() {
	int i;
	const Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	const Uint8 * keyboard = SDL_GetKeyboardState(&numKeys);

	oldMouseX = mouseX;
	oldMouseY = mouseY;

	keys = new Uint8[numKeys];
	prevKeys = new Uint8[numKeys];

	memcpy(keys, keyboard, sizeof(keyboard[0]) * numKeys);
	memset(prevKeys, 0, sizeof(prevKeys[0]) * numKeys);

	for (i = 1; i <= 3; i++) {
		prevMouseButtons[i - 1] = 0;
		mouseButtons[i - 1] = mouseState & SDL_BUTTON(i);
	}
}

//***************
// eInput::~eInput
//***************
eInput::~eInput() {
	delete[] keys;
	delete[] prevKeys;
}

//***************
// eInput::Update
// reads the current state of the keyboard and mouse and saves the previous state
//***************
void eInput::Update() {
	oldMouseX = mouseX;
	oldMouseY = mouseY;

	const Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	const Uint8 * keyboard = SDL_GetKeyboardState(&numKeys);

	memcpy(prevKeys, keys, sizeof(prevKeys[0]) * numKeys);
	memcpy(keys, keyboard, sizeof(keys[0]) * numKeys);

	for (int i = 1; i <= 3; i++) {
		prevMouseButtons[i - 1] = mouseButtons[i - 1];
		mouseButtons[i - 1] = mouseState & SDL_BUTTON(i);
	}
}

//***************
// eInput::KeyHeld
// returns 1 if the key is currently being pressed, 0 if not, -1 if invalid key
//***************
int eInput::KeyHeld(int key) const {
	if (key < 0 || key > numKeys)
		return -1;

	return keys[key];
}

//***************
// eInput::KeyPressed
// returns 1 if the key changed state from released to pressed, 0 otherwise, -1 if invalid key
//***************
int eInput::KeyPressed(int key) const {
	if (key < 0 || key > numKeys)
		return -1;

	return (keys[key] & ~prevKeys[key]);
}

//***************
// eInput::KeyReleased
// returns 1 if the key changed state from pressed to released, 0 otherwise, -1 if invalid key
//***************
int eInput::KeyReleased(int key) const {
	if (key < 0 || key > numKeys)
		return -1;

	return (prevKeys[key] & ~keys[key]);
}

//***************
// eInput::MouseHeld
// returns 1 if the button is currently being pressed, 0 if not, -1 if invalid button
//***************
int eInput::MouseHeld(int button) const {
	if (button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT)
		return -1;

	return mouseButtons[button - 1];
}

//***************
// eInput::MousePressed
// returns 1 if the button changed state from released to pressed, 0 if not, -1 if invalid button
//***************
int eInput::MousePressed(int button) const {
	if (button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT)
		return -1;

	return (mouseButtons[button - 1] & ~prevMouseButtons[button - 1]);
}

//***************
// eInput::MouseReleased
// returns 1 if the button changed state from pressed to released, 0 if not, -1 if invalid button
//***************
int eInput::MouseReleased(int button) const {
	if (button < 1 || button > 3)
		return -1;

	return (prevMouseButtons[button - 1] & ~mouseButtons[button - 1]);
}

//***************
// eInput::MouseMoved
// returns true if the mouse x or y value has changed, false otherwise
//***************
bool eInput::MouseMoved() const {
	return (mouseX != oldMouseX || mouseY != oldMouseY);
}

//***************
// eInput::GetMouseX
// returns the current x-position of the mouse cursor within the focused window
//***************
int eInput::GetMouseX() const {
	return mouseX;
}

//***************
// eInput::GetMouseY
// returns the current y-position of the mouse cursor within the focused window
//***************
int eInput::GetMouseY() const {
	return mouseY;
}

//***************
// eInput::HideCursor
// returns the current y-position of the mouse cursor
//***************
void eInput::HideCursor(bool hide) const {
	if (hide)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);
}


