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
#include "CreatePrefabStrategies.h"
#include "sHero.h"

//***************
// eCreateEntityPrefabUser::CreatePrefab
//***************
bool eCreateEntityPrefabUser::CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) {
	if (prefabShortName == "Entity")
		newPrefab = std::make_shared<eEntity>();
	else if (prefabShortName == "sHero" || prefabShortName == "sArcher")
		newPrefab = std::make_shared<sHero>();
	else
		return false;

	bool success = true;
	success = newPrefab->AddRenderImage(	spawnArgs.GetString("spriteFilename", ""), 
											spawnArgs.GetVec3("renderBlockSize", "0 0 0"), 
											spawnArgs.GetInt("initialSpriteFrame", "0"), 
											spawnArgs.GetVec2("renderImageOffset", "0 0"), 
											spawnArgs.GetBool("playerSelectable", "0")
										);

	success = newPrefab->AddAnimationController(spawnArgs.GetString("animationController", ""));

	eQuat minMax = spawnArgs.GetVec4("localBounds", "1 1 0 0");					// default empty bounds
	eBounds localBounds(eVec2(minMax.x, minMax.y), eVec2(minMax.z, minMax.w));
	success = newPrefab->AddCollisionModel(localBounds, spawnArgs.GetVec2("colliderOffset", "0 0"), spawnArgs.GetBool("collisionActive", "0"));
	success = newPrefab->AddMovementPlanner(spawnArgs.GetFloat("movementSpeed", "0"));
	newPrefab->SetStatic(spawnArgs.GetBool("isStatic", "1"));
	return success;
}