//**********************************************
//*
//*Copyright Thomas Freehill July 29 2016
//*This is just garbage code I'm using to learn
//*how I'd like to structure my future code
//*and to get back in the rhythm of 
//*readability and perfomance
//*
//**********************************************

#include "Game.h"
#include <Windows.h>

// DEBUG: no using SDL_main
#undef main

 int main() {
	eGame::ErrorCode initCode;

	initCode = game.Init();
	if (initCode != eGame::INIT_SUCCESS) {
		game.Shutdown(initCode);
		return 0;
	}

	while (game.Run());

	game.Shutdown(eGame::INIT_SUCCESS);

	return 0;
}




