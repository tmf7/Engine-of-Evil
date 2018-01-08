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

//*************************************************************//
// Original Copyright (C) Thomas Matthew Freehill July 29 2016 //
//*************************************************************//
#include "GameLocal.h"

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

	if (!gameLocal.InitSystem()) {
		gameLocal.ShutdownSystem();
		return 1;
	}

	gameLocal.Run();

	return 0;
}




