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
#include "GameLocal.h"
#include "CreatePrefabStrategies.h"

using namespace logic;

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
eGameLocal					gameLocal;					// the user's static game instance
eGame *						game = &logic::gameLocal;	// statically pointed at an eGameLocal

// FIXME: change this logic a bit (different eCamera object use, no eMap::Init, just load one/assign a viewCamera and renderer/window)
//***********************
// eGameLocal::Init
//***********************
bool eGameLocal::Init() {
	auto windowSize = gameLocal.GetRenderer().ViewArea();
	camera.Init(&map, eVec2((float)windowSize.w, (float)windowSize.h), vec2_zero);

	gameLocal.GetEntityPrefabManager().SetCreatePrefabStrategy(std::make_shared<eCreateEntityPrefabUser>());

	map.SetViewCamera(&camera);
	player.SetMap(&map);

	if (!map.LoadMap("Graphics/Maps/EvilTown2.emap")) {
		eErrorLogger::ErrorPopupWindow("MAP INIT FAILURE");
		return false;
	}
	return true;
}

//***********************
// eGameLocal::Update
//***********************
void eGameLocal::Update() {
	player.Think();
	map.EntityThink();
	camera.Think();

	map.Draw();
	player.Draw();
	map.DebugDraw();
	player.DebugDraw(camera.GetDebugRenderTarget());
}