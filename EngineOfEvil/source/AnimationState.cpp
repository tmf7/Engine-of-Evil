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
#include "AnimationState.h"
#include "Game.h"

using namespace evil;

ECLASS_DEFINITION(eStateNode, eAnimationState)

//*********************
// eAnimationState::eAnimationState
//*********************
eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed)
	: animation(animation) {
	this->speed = (speed > 0.0f ? speed : 1.0f);
	this->name = name;

	currentFrame = &animation->GetFrame(0);
	duration = (animation->Duration() / speed) + game->GetFixedTime();	// BUGFIX: + FixedTime() prevents skipping the last animation frame during playback
	nameHash = std::hash<std::string>()(name);
}

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	NextFrame(*animation);
}