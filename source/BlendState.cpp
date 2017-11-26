#include "BlendState.h"
#include "Game.h"

//*********************
// eBlendState::Update
// decides which of eBlendState::animations 
// should being playing
//*********************
void eBlendState::Update() {
	eVec2 controllerNode(*xBlendParameter, (blendMode == AnimationBlendMode::SIMPLE_1D ? 0.0f : *yBlendParameter));
	float lowestDistSqr = FLT_MAX;
	int bestAnimationIndex = -1;

	// blendNode weights based on squared-distance to observed eAnimationController::floatParameters
	for (int i = 0; i < blendNodes.size(); ++i) {
		float distSqr = (blendNodes[i] - controllerNode).LengthSquared();
		if (distSqr < lowestDistSqr) {
			lowestDistSqr = distSqr;
			bestAnimationIndex = i;
		}
	}

	if (bestAnimationIndex != currentAnimationIndex)
		SwapAnimation(bestAnimationIndex);

	NextFrame(*animations[currentAnimationIndex]);
}