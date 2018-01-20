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
#include "RenderTarget.h"
#include "RenderImageIsometric.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "AnimationController.h"
#include "Game.h"

using namespace logic;

//***************
// eCreateEntityPrefabUser::CreatePrefab
//***************
bool eCreateEntityPrefabUser::CreatePrefab( std::shared_ptr< eEntity > & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs ) {
	if ( prefabShortName == "Entity" ) {
		newPrefab = std::make_shared< eEntity >();

	} else if ( prefabShortName == "sHero" || prefabShortName == "sArcher" ) {
		newPrefab = std::make_shared< sHero >();

	} else {
		std::string message;
		message = "Invalid prefabShortName: ";
		message += prefabShortName;
		eErrorLogger::LogError( message.c_str(), __FILE__, __LINE__ );
		return false;
	}

	// eRenderImage
	std::shared_ptr< eImage > initialImage = nullptr;
	if ( game->GetImageManager().LoadAndGet( spawnArgs.GetString("spriteFilename", ""), initialImage) ) {
		const eVec3 renderBlockSize = spawnArgs.GetVec3("renderBlockSize", "0 0 0");

		if ( renderBlockSize != vec3_zero ) {
			newPrefab->AddComponent< eRenderImageIsometric >(	newPrefab.get(),
																initialImage,
																renderBlockSize, 
																spawnArgs.GetInt( "initialSpriteFrame", "0" ), 
																spawnArgs.GetVec2( "renderImageOffset", "0 0" ), 
																spawnArgs.GetBool( "playerSelectable", "0" )
															 );
		} else {
			newPrefab->AddComponent< eRenderImageBase >(	newPrefab.get(),
															initialImage,
															spawnArgs.GetInt( "initialSpriteFrame", "0" ), 
															spawnArgs.GetVec2( "renderImageOffset", "0 0" ), 
															spawnArgs.GetBool( "playerSelectable", "0" )
														 );
		}
	}

	// eAnimationController
	std::shared_ptr< eAnimationController > prefabAnimationController = nullptr;
	if ( game->GetAnimationControllerManager().LoadAndGet( spawnArgs.GetString("animationController", "" ), prefabAnimationController ) ) {
		newPrefab->AddComponent< eAnimationController >( newPrefab.get(), *prefabAnimationController );
	}

	// eCollisionModel
	eQuat minMax = spawnArgs.GetVec4( "localBounds", "1 1 0 0" );					// default empty bounds
	eBounds localBounds (eVec2( minMax.x, minMax.y ), eVec2( minMax.z, minMax.w ) );
	if ( !localBounds.IsEmpty() ) {
		newPrefab->AddComponent< eCollisionModel >( newPrefab.get(), 
												    localBounds,
												    spawnArgs.GetVec2( "colliderOffset", "0 0" ),
												    spawnArgs.GetBool( "collisionActive", "0" )
												  );
	}
	
	// eMovementPlanner
	const float movementSpeed = spawnArgs.GetFloat( "movementSpeed", "0" );
	if ( movementSpeed ) {
		newPrefab->AddComponent< eMovementPlanner >( newPrefab.get(), 
												     movementSpeed 
												   );
	}

	newPrefab->SetStatic( spawnArgs.GetBool( "isStatic", "1" ) );
	return true;
}