#include "AnimationState.h"
#include "Game.h"

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	time += (float)game.GetFixedTime();
	if (time > duration) { 
		switch (animation->loop) {
			case AnimationLoopState::ONCE:		time = duration; break;
			case AnimationLoopState::REPEAT:	time = 0.0f; break;
		}
	}

	for (auto & frame : animation->frames) {
		auto animTime = frame.normalizedTime * duration;
		if (animTime <= time)
			currentFrame = &frame;
		else
			break;
	}

	auto & targetRenderImage = stateMachine->Owner()->RenderImage();
	targetRenderImage.SetImage(currentFrame->imageManagerIndex);
	targetRenderImage.SetImageFrame(currentFrame->subframeIndex);
}