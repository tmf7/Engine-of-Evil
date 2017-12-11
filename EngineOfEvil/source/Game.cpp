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
#include "CreatePrefabStrategies.h"

eGame game;

//****************
// eGame::Init
//****************
bool eGame::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		EVIL_ERROR_LOG.ErrorPopupWindow("SDL INIT FAILURE");
		return false;
	}

	if (!EVIL_ERROR_LOG.Init())			// has its own error popup call
		return false;

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

	camera.Init();

	if (!entityPrefabManager.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("ENTITY PREFAB MANAGER INIT FAILURE");
		return false;
	}

	entityPrefabManager.SetCreatePrefabStrategy(std::make_shared<eCreateEntityPrefabUser>());			// FIXME: move this to an external, user-initialization function, not Engine Initialization

	if (!map.Init()) {
		EVIL_ERROR_LOG.ErrorPopupWindow("MAP INIT FAILURE");
		return false;
	}
	
	gameTime = SDL_GetTicks();

	return true;
}

//****************
// eGame::Shutdown
//****************
void eGame::Shutdown() {
	FreeAssets();
	SDL_Quit();
}

//****************
// eGame::FreeAssets
//****************
void eGame::FreeAssets() {
	renderer.Free();
}

//****************
// eGame::AddEntity
// finds the first unused slot in game::entities to move param entity
// and assigns it a spawnID,
// returns the new spawnID index within game::entities
// returns -1 if something went wrong
//****************
int eGame::AddEntity(std::unique_ptr<eEntity> && entity) {
	int spawnID = 0;
	for (auto & entitySlot : entities) {
		if (entitySlot == nullptr) {
			entity->spawnedEntityID = spawnID;
			entity->spawnName = entity->spawnArgs.GetString("prefabShortName", TO_STRING(eEntity));
			entity->spawnName += "_" + spawnID;
			entitySlot = std::move(entity);
			return spawnID;
		} else {
			++spawnID;
		}
	}

	if (spawnID == entities.size()) {
		entity->spawnedEntityID = spawnID;
		entities.emplace_back(std::move(entity));
		return spawnID;
	}
	return -1;
}

//****************
// eGame::ClearAllEntities
//****************
void eGame::ClearAllEntities() {
	entities.clear();
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
	renderer.DrawOutlineText(fraps.c_str(), vec2_zero, redColor, true, RENDERTYPE_STATIC);
}

//****************
// eGame::Run
//****************
bool eGame::Run() {	
	Uint32 startTime = SDL_GetTicks();

	// TODO: poll events in a separate eWindow class
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return false;
	}
	
	// system updates
	input.Update();

	player.Think();

	for (auto & entity : entities) {
		entity->UpdateComponents();
		entity->Think();
	}

	camera.Think();

	renderer.Clear();
	map.Draw();
	player.Draw();

	// DEBUG: all debug information is an overlay on their respective target texture [camera|overlay]
	for (auto & entity : entities)
		entity->DebugDraw();		

	map.DebugDraw();
	player.DebugDraw();

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
	return true;
}
