#include "AnimationState.h"
#include "Game.h"

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	time += game.GetFixedTime();
	normalizedTime = (float)time / (float)duration;
	if (normalizedTime >= 1.0f)
		;	// stop updating time, reset to 0 to loop again, or start subtracting from time until 0
}

//*********************
// eAnimationState::SetNormalizedTime
//*********************
void eAnimationState::SetNormalizedTime(float newNormalizedTime) {
	normalizedTime = newNormalizedTime;
	time = (Uint32)eMath::NearestInt(normalizedTime * (float)duration);
}