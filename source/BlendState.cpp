#include "BlendState.h"
#include "Game.h"

//*********************
// eBlendState::eBlendState
//*********************
eBlendState::eBlendState(const std::string & name, int numAnimations, float * xBlendParameter, float * yBlendParameter, AnimationBlendMode blendMode, float speed)
	: xBlendParameter(xBlendParameter),
	  yBlendParameter(yBlendParameter),
	  blendMode(blendMode),
	  currentAnimationIndex(0) {
	this->speed = speed;
	this->name = name;

	nameHash = std::hash<std::string>()(name);
	blendNodesHash.ClearAndResize(numAnimations);
	animations.reserve(numAnimations);
	blendNodes.reserve(numAnimations);
}

//*********************
// eBlendState::Init
// DEBUG: called after all BlendNodes have been added to *this
//*********************
void eBlendState::Init() {
	currentFrame = &animations[currentAnimationIndex]->GetFrame(0);
	duration = (animations[currentAnimationIndex]->Duration() * speed) + (float)game.GetFixedTime();	// BUGFIX: + FixedTime() prevents skipping the last animation frame during playback
}

//*********************
// eBlendState::AddBlendNode
// assigns the values to which the eAnimationController 
// stateMachine's paramaters will be compared
// returns true if the animation is already loaded and can be added
// with corresponding blendNode (x,y) values
// returns false if the animation hasn't been loaded yet
// DEBUG: the same animation can be added multiple times with
// different (x,y) blendNode values
//*********************
bool eBlendState::AddBlendNode(const std::string & animationName, float xPosition, float yPosition) {
	const int index = animations.size();
	auto & animation = game.GetAnimationManager().Get(animationName.c_str());
	if (!animation->IsValid())
		return false;

	animations.emplace_back(animation);
	blendNodesHash.Add(animation->GetNameHash(), index);
	blendNodes.emplace_back(eVec2(xPosition, yPosition));
	return true;
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
	for (size_t i = 0; i < blendNodes.size(); ++i) {
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