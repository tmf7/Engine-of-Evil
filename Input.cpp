#include "Input.h"

//***************
// eInput::Init
//***************
void eInput::Init() {
	int i;
	const Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	const Uint8 * keyboard = SDL_GetKeyboardState(&numKeys);

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
	int i;
	const Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	const Uint8 * keyboard = SDL_GetKeyboardState(&numKeys);

	memcpy(prevKeys, keys, sizeof(prevKeys[0]) * numKeys);
	memcpy(keys, keyboard, sizeof(keys[0]) * numKeys);

	for (i = 1; i <= 3; i++) {
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
	if (button < 1 || button > 3)
		return -1;

	return mouseButtons[button - 1];
}

//***************
// eInput::MousePressed
// returns 1 if the button changed state from released to pressed, 0 if not, -1 if invalid button
//***************
int eInput::MousePressed(int button) const {
	if (button < 1 || button > 3)
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


