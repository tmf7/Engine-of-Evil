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
#include "sHero.h"
#include "Map.h"

void sHero::Think() {
	auto & velocity = collisionModel->GetVelocity();
	eVec2 facingDirection;
	if (velocity != vec2_zero) {
		facingDirection = velocity.Normalized();
		eMath::CartesianToIsometric(facingDirection.x, facingDirection.y);
		facingDirection.Normalize();
		oldFacingDirection = facingDirection;
	} else {
		facingDirection = oldFacingDirection * 0.25f;
	}
			
	animationController->SetFloatParameter(xSpeedParameterHash, facingDirection.x);
	animationController->SetFloatParameter(ySpeedParameterHash, facingDirection.y);
	animationController->SetFloatParameter(magnitudeParameterHash, facingDirection.Length());
}

//***************
// sHero::SpawnCopy
// copies a prefab sHero and adds unique details
//***************
bool sHero::SpawnCopy(eMap * onMap, const eVec3 & worldPosition) {
	auto & newHero = std::make_unique<sHero>(*this);
	newHero->map = onMap;
	newHero->SetZPosition(worldPosition.z);
	newHero->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	return (onMap->AddEntity(std::move(newHero)) >= 0);
}