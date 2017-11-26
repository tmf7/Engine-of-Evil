#include "BlendState.h"
#include "Game.h"

//*********************
// eBlendState::Update
// TODO: re-calculate duration when the animation swaps to a different index
// and use the SAVED normalized time to set the new time
// TODO: select an animation based on 1 or 2 (not more) float_type parameters
// compared to eAnimationController stateMachine parameters (as float *) 
// [NOTE]: at least one float must be selected, if not in load-file, then default to first floatParameter)

// TODO: selection based on lowest squaredDistance from controllerParam(s) position to blendNode param(s) position
// TODO: blendNode params will be 1 or 2 floats PER eAnimation in the eBlendState (directly proportional to controllerParams being tracked)
// and associated fixed-values (for getting the distance to)
//*********************
void eBlendState::Update() {
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