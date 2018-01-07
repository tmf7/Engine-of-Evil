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
#include "StateNode.h"
#include "Game.h"

using namespace evil::animation;

//*********************
// eStateNode::NextFrame
// updates the target eRenderImage-derived object
// according to param animation
//*********************
void eStateNode::NextFrame(const eAnimation & animation) {
	time += (float)game->GetDeltaTime();
	if (time > duration) { 
		switch (animation.loop) {
			case AnimationLoopState::ONCE:		time = duration; break;
			case AnimationLoopState::REPEAT:	time = 0.0f; break;
		}
	}

	// the price-is-right for which animation frame should be active
	for (auto & frame : animation.frames) {
		auto animTime = frame.normalizedTime * duration;
		if (animTime <= time)
			currentFrame = &frame;
		else
			break;
	}

	
	auto & targetRenderImage = stateMachine->Owner()->RenderImage();
	if (&targetRenderImage != nullptr) {		// BUGFIX: removes load and run-time dependence between eAnimationController and eRenderImageBase-derived instances
		targetRenderImage.SetImage(currentFrame->imageManagerIndex);
		targetRenderImage.SetImageFrame(currentFrame->subframeIndex);
	}
}

