#include "AnimationState.h"
#include "Game.h"

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	time += game.GetFixedTime();
	if (time > duration) { 
		switch (loop) {
			case AnimationLoopState_t::ONCE: {
				time = duration; 
				break;
			}
			case AnimationLoopState_t::REPEAT: {
				time = 0.0f; 
				currentFrame = 0;		// FIXME: not necessarily the case, if the first frame n_time > 0, this should linger on the last frame a bit longer
				break;
			}
		}
	}
	normalizedTime = (float)time / (float)duration;


	// FIXME: make sure not out of bounds
	// FIXME: doesn't work for SetNormalizedTime functionality (jumping around in animations)
	// SOLUTION: that's where interval checking came in (frameIndex * frameInterval, min + frameInterval)
	// SOLUTION: normalizedTime +/- frameInterval && range-based loop over frames...except the frameInterval isn't in normalized time
	// ... n_time can be converted to time via duration... and perhaps should be because it s multiplication

	for (auto & frame : animation->frames) {
		auto animTime = frame.normalizedTime * (float)duration;	// or animation->duration ?
		time; animation->frameInterval;
	}

	if ((currentFrame + 1)->normalizedTime <= normalizedTime)
		++currentFrame;


	// FIXME: if currentFrame == lastFrame, then this wont REPEAT a intended
	// SOLUTION(~): start from i == 0
	// FIXME: if first frame has n_time > first rangeMinMax then it may never hit ???

	// SOLUTION(?): set the currentFrame according to
	// the animation.frame.normalizedTime CLOSEST to eAnimationState.normalized time
	// (without going over! and independent of current currentFrame state)
}

//*********************
// eAnimationState::SetNormalizedTime
//*********************
void eAnimationState::SetNormalizedTime(float newNormalizedTime) {
	normalizedTime = newNormalizedTime;
	time = (Uint32)eMath::NearestInt(normalizedTime * (float)duration);
}