#ifndef EVIL_INPUT_H
#define EVIL_INPUT_H

#include "Definitions.h"

//*************************************************************************
//							eInput
// TODO: fix the mouse indexing, currently SDL_BUTTON_LEFT/MIDDLE/RIGHT are used
// which are 1/2/3 respectively, which does not align nicely with mousButtons array.
// Therefore, currently the hack of decreasing the queried button by 1 is used in
// MouseHeld/MousePressed/MouseReleased, which could break the code if more buttons
// are ever used
//*************************************************************************
class eInput {
public:

						eInput();
						~eInput();

	bool				Init();
	void				Update();

	int					KeyHeld(int key) const;
	int					KeyPressed(int key) const;
	int					KeyReleased(int key) const;

	int					MouseHeld(int button) const;
	int					MousePressed(int button) const;
	int					MouseReleased(int button) const;
	int					GetMouseX() const;
	int					GetMouseY() const;
	void				HideCursor(bool hide = true) const;

private:

	Uint8 *				keys;
	Uint8 *				prevKeys;
	int					numKeys;

	int					mouseButtons[3];
	int					prevMouseButtons[3];
	int					mouseX;
	int					mouseY;
};

//***************
// eInput::eInput
//***************
inline eInput::eInput() : keys(nullptr), prevKeys(nullptr) {
}

#endif /* EVIL_INPUT_H */

