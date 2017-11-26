#ifndef EVIL_ANIMATION_STATE_H
#define EVIL_ANIMATION_STATE_H

#include "StateNode.h"

class eAnimationController;

//******************************
//		eAnimationState
// used by eAnimationController
// as part of its state machine
// only controls one animation
// see also: eBlendState
//*******************************
class eAnimationState : public eStateNode {
public:

	friend class eAnimationController;		// sole access to Update

public:

											eAnimationState(const std::string & name, 
															const std::shared_ptr<eAnimation> & animation, 
															float speed = 1.0f);

	virtual int								GetClassType() const override { return CLASS_ANIMATIONSTATE; }

private:

	virtual void							Update() override;

private:

	std::shared_ptr<eAnimation>				animation;				// which animation this state plays
};

//*********************
// eAnimationState::eAnimationState
//*********************
inline eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed)
	: animation(animation) {
	this->speed = speed;
	this->name = name;

	currentFrame = &animation->GetFrame(0);
	duration = animation->Duration() * speed;
	nameHash = std::hash<std::string>()(name);
}

#endif /* EVIL_ANIMATION_STATE_H */