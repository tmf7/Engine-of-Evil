#include "BlendState.h"
#include "Game.h"

//*********************
// eBlendState::eBlendState
//*********************
eBlendState::eBlendState(const std::string & name, const std::vector<std::shared_ptr<eAnimation>> & animations, float * xBlendParameter, float * yBlendParameter, AnimationBlendMode blendMode, float speed)
	: animations(animations),
	  xBlendParameter(xBlendParameter),
	  yBlendParameter(yBlendParameter),
	  blendMode(blendMode) {
	this->speed = speed;
	this->name = name;

	currentFrame = &animations[0]->GetFrame(0);
	duration = (animations[0]->Duration() * speed) + (float)game.GetFixedTime();	// BUGFIX: + FixedTime() prevents skipping the last animation frame during playback

	nameHash = std::hash<std::string>()(name);

	blendNodes.assign(animations.size(), vec2_zero);
	blendNodesHash.ClearAndResize(animations.size());
	for (size_t index = 0; index < animations.size(); ++index)
		blendNodesHash.Add(animations[index]->NameHash(), index);
}

//*********************
// eBlendState::SwapAnimation
// switches which animation this state is playing
// using the same normalized time
//*********************
void eBlendState::SwapAnimation(int animationIndex) {
	const float normalizedTime = (time / duration);
	currentAnimationIndex = animationIndex;
	duration = (animations[currentAnimationIndex]->Duration() * speed) + (float)game.GetFixedTime();	// BUGFIX: + FixedTime() prevents skipping the last animation frame during playback
	time = normalizedTime * duration;
}

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