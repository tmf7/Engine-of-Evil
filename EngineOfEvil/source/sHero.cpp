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

using namespace evil;

ECLASS_DEFINITION(evil::eEntity, sHero)

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
// copies a prefab sHero and adds it to param onMap
//***************
bool sHero::SpawnCopy(eMap * onMap, const eVec3 & worldPosition) {
	return (onMap->AddEntity(std::make_unique<sHero>(*this), worldPosition) >= 0);
}