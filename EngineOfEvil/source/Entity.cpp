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

	// TODO(!): don't add eEntities to the map add eGameObjects,
	// OR: add eGameObjects to an eGameState (not eMap anymore because they change while objects may need more persistance)
	// or even just the eGame again...ALTHOUGH the .emap file contains the spawns of eEntities....that doesn't speak to the Scene at large,
	// namely any GUIs or (well sound/music cues would also be on the map)...

	// FIXME: the present separation b/t eGameObjects and eEntities is that eEntity can be prefabed as an eResource
	// SOLUTION: collapse eEntity into eGameObject and make eEntityPrefabManager into ePrefabManager : public eResourceManager<eGameObject>
	// ... which would make eGameObject : public eClass, public eResource;
	// SOLUTION(better?): make ePrefabManager : public eResourceManager<eGameObjectDef> that only stores spawnArgs dictionaries (w/o localized Spawn info)
	// then update the RTTI to allow dynamic calling of a classname::CreateInstance(), and/or classname::Spawn() that uses those spawnArgs

	// SOLUTION: and make eGameObject::DebugDraw non-virtual and just call eComponent::DebugDraw() for all its non-nullptr components


	auto & newEntity = std::make_unique<eEntity>(*this);
	newEntity->collisionModel = newEntity->GetComponent<eCollisionModel>();
	newEntity->animationController = newEntity->GetComponent<eAnimationController>();
	newEntity->movementPlanner = newEntity->GetComponent<eMovementPlanner>();
	newEntity->renderImage = newEntity->GetComponent<eRenderImageIsometric>();
	return (onMap->AddEntity(std::move(newEntity), worldPosition) >= 0);
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw(eRenderTarget * renderTarget) {
	if (game->debugFlags.RENDERBLOCKS && animationController != nullptr)
		game->GetRenderer().DrawIsometricPrism(renderTarget, lightBlueColor, renderImage->GetRenderBlock());

//	if (game->debugFlags.WORLDCLIPS)
//		game->GetRenderer().DrawCartesianRect(lightBlueColor, renderImage->GetWorldClip(), false);

	// TODO: better visual difference b/t cells occupied by both renderImage and collisionmodel
	if (game->debugFlags.GRID_OCCUPANCY) {
		if (renderImage != nullptr) {
			for (auto & cell : renderImage->Areas()) {
				game->GetRenderer().DrawIsometricRect(renderTarget, yellowColor, cell->AbsBounds());
			}
		}

		if (collisionModel != nullptr) {
			for (auto & cell : collisionModel->Areas()) {
				game->GetRenderer().DrawIsometricRect(renderTarget, lightBlueColor, cell->AbsBounds());
			}
		}
	}

	if (game->debugFlags.COLLISION && collisionModel != nullptr)
		game->GetRenderer().DrawIsometricRect(renderTarget, yellowColor, collisionModel->AbsBounds());

	if (movementPlanner != nullptr)
		movementPlanner->DebugDraw();
}