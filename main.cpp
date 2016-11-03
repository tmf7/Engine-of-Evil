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
//__stdcall
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	Game evil;
	Game::ErrorCode initCode;
	
	initCode = evil.Init();
	if (initCode != Game::INIT_SUCCESS) {
		evil.Shutdown(initCode);
		return 0;
	}

	while (evil.Run());

	evil.Shutdown(Game::INIT_SUCCESS);

	return 0;
}




