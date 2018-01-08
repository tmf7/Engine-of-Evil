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
#include "Map.h"
#include "RenderTarget.h"
#include "RenderImageIsometric.h"
#include "CollisionModel.h"
#include "Movement.h"

using namespace evil;

ECLASS_DEFINITION(eGameObject, eEntity)

//***************
// eEntity::SpawnCopy
// copies a *this and adds it to param onMap
// DEBUG: *this is typically a prefab from eEntityPrefabManager::SpawnInstance
//***************
bool eEntity::SpawnCopy(eMap * onMap, const eVec3 & worldPosition) {
	return (onMap->AddEntity(std::make_unique<eEntity>(*this), worldPosition) >= 0);
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw(eRenderTarget * renderTarget) {
	if (game->debugFlags.RENDERBLOCKS && &animationController != nullptr)
		game->GetRenderer().DrawIsometricPrism(renderTarget, lightBlueColor, renderImage.GetRenderBlock());

//	if (game->debugFlags.WORLDCLIPS)
//		game->GetRenderer().DrawCartesianRect(lightBlueColor, renderImage->GetWorldClip(), false);

	// TODO: better visual difference b/t cells occupied by both renderImage and collisionmodel
	if (game->debugFlags.GRID_OCCUPANCY) {
		if (&renderImage != nullptr) {
			for (auto & cell : renderImage.Areas()) {
				game->GetRenderer().DrawIsometricRect(renderTarget, yellowColor, cell->AbsBounds());
			}
		}

		if (&collisionModel != nullptr) {
			for (auto & cell : collisionModel.Areas()) {
				game->GetRenderer().DrawIsometricRect(renderTarget, lightBlueColor, cell->AbsBounds());
			}
		}
	}

	if (game->debugFlags.COLLISION && &collisionModel != nullptr)
		game->GetRenderer().DrawIsometricRect(renderTarget, yellowColor, collisionModel.AbsBounds());

	if (&movementPlanner != nullptr)
		movementPlanner.DebugDraw();
}