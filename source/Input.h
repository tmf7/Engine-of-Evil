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
	void				HideCursor(bool hide = true) const;

	virtual int			GetClassType() const override { return CLASS_INPUT; }

private:

	Uint8 *				keys;
	Uint8 *				prevKeys;
	int					numKeys;

	int					mouseButtons[3];
	int					prevMouseButtons[3];
	int					mouseX;
	int					mouseY;
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

