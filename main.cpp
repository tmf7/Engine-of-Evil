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
#include "Heap.h"

 int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	eGame::ErrorCode initCode;

	/////////////////////
	auto max = [](int a, int b) { return a > b; };
	eHeap<int, decltype(max)> test(max);
	//////////////////////
	
	initCode = game.Init();
	if (initCode != eGame::INIT_SUCCESS) {
		game.Shutdown(initCode);
		return 0;
	}

	while (game.Run());

	game.Shutdown(eGame::INIT_SUCCESS);

	return 0;
}




