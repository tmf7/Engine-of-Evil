// Globally visible user-input debugging functions
// how can these be issued/called from anywhere in the code?
// flush a command queue during input update?

#if 0

#include "Game.h"

//******************
// ClearKnownMap_Cmd
// TODO: this is just a general idea of such user-cmd functions
// the final versions shouldn't need complex parameter specifications
//******************
void ClearKnownMap_Cmd(eAI * ai) {
	eInput & input = game.GetInput();
	if (game.debugFlags.KNOWN_MAP_CLEAR && input->KeyPressed(SDL_SCANCODE_R))
		ai->KnownMap().ClearAllCells();
}

#endif