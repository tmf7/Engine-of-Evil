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

 int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	eGame evil;
	eGame::ErrorCode initCode;
	
	initCode = evil.Init();
	if (initCode != eGame::INIT_SUCCESS) {
		evil.Shutdown(initCode);
		return 0;
	}

	while (evil.Run());

	evil.Shutdown(eGame::INIT_SUCCESS);

	return 0;
}




