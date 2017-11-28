#include "AnimationState.h"
#include "Game.h"

//*********************
// eAnimationState::eAnimationState
//*********************
eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed)
	: animation(animation) {
	this->speed = speed;
	this->name = name;

	currentFrame = &animation->GetFrame(0);
	duration = (animation->Duration() * speed) + (float)game.GetFixedTime();	// BUGFIX: + FixedTime() prevents skipping the last animation frame during playback
	nameHash = std::hash<std::string>()(name);
}

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	NextFrame(*animation);
}