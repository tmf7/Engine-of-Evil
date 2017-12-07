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

// DEBUG: not using SDL_main
#undef main

 int main() {
	// TODO: possibly create a single function call here
	// EngineOfEvil.Start();
	// that initializes the engine critical systems, and runs on a loop
	// which (prior to the main loop) calls all user-defined Start() functions.
	// Then when the loop ends it automatically calls Shutdown in its dtor
	// all the while logging errors (if any)

	//-------------------------------------------------------------------------
	// TODO: the goal is the have a user create a small main() entry point
	// then freely define classes that extend/use the EngineOfEvil functionality
	//-------------------------------------------------------------------------

	// TODO: compile EngineOfEvil as a library to link against
	// TODO: make all headers clean so the library's implementation isn't easily messed with
	// and for faster testing compile times

	if (!game.Init()) {
		game.Shutdown();
		return 1;
	}

	while (game.Run())
		;

	game.Shutdown();

	return 0;
}




