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
#include "Game.h"

//****************
// eGame::InitSystem
//****************
bool eGame::InitSystem() {

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		EVIL_ERROR_LOG.ErrorPopupWindow("SDL INIT FAILURE");
		return false;
	}

	if (!EVIL_ERROR_LOG.Init())			// has its own error popup call
		;//	return false;				// no consequences if this is running from DVD-ROM

	if (!renderer.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("RENDERER INIT FAILURE");
		return false;
	}

	if (!imageManager.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("IMAGE MANAGER INIT FAILURE");
		return false;
	}

	if (!animationManager.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("ANIMATION MANAGER INIT FAILURE");
		return false;
	}

	if (!animationControllerManager.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("ANIMATION CONTROLER MANAGER INIT FAILURE");
		return false;
	}

	if (!audio.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("AUDIO INIT FAILURE");
		return false;
	}

	try {
		input.Init();
	} catch (...) {
		EVIL_ERROR_LOG.ErrorPopupWindow("INPUT INIT FAILURE");
		throw;
	}

	if (!entityPrefabManager.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("ENTITY PREFAB MANAGER INIT FAILURE");
		return false;
	}

	SetFixedFPS(defaultFPS);
	gameTime = SDL_GetTicks();
	return Init();
}

//****************
// eGame::ShutdownSystem
//****************
void eGame::ShutdownSystem() {
	audio.Shutdown();
	renderer.Shutdown();
	SDL_Quit();
}


//****************
// eGame::DrawFPS
// add fps text to the renderPool
//****************
void eGame::DrawFPS() {
	std::string fraps = "FPS: ";
	fraps += std::to_string(fixedFPS);
	fraps += "/";
	fraps += std::to_string(GetDynamicFPS());
	renderer.DrawOutlineText(renderer.GetDebugOverlayTarget(), fraps.c_str(), vec2_zero, redColor, true);
}

//****************
// eGame::Run
//****************
void eGame::Run() {	
	isRunning = true;
	while (isRunning) {
		Uint32 startTime = SDL_GetTicks();

		// system updates
		input.Update();
		Update();

		// draw static debug information
		if (debugFlags.FRAMERATE)
			DrawFPS();

		renderer.Flush();
		renderer.Show();

		// frame-rate governing delay
		gameTime = SDL_GetTicks();
		deltaTime = gameTime - startTime;

		// DEBUG: breakpoint handling
		if (deltaTime > 1000)
			deltaTime = frameTime;

		// DEBUG: delta time of this last frame is not used as the global update interval,
		// instead the globally available update interval is fixed to frameTime
		deltaTime <= frameTime ? SDL_Delay(frameTime - deltaTime) : SDL_Delay(deltaTime - frameTime);
	}
	Shutdown();
	ShutdownSystem();
}
