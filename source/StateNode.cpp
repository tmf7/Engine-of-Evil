#include "StateNode.h"
#include "Game.h"

//*********************
// eStateNode::NextFrame
// updates the target eRenderImage
// according to param animation
//*********************
void eStateNode::NextFrame(const eAnimation & animation) {
	time += (float)game.GetFixedTime();
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
	targetRenderImage.SetImage(currentFrame->imageManagerIndex);
	targetRenderImage.SetImageFrame(currentFrame->subframeIndex);
	targetRenderImage.Update();
}

