#ifndef EVIL_ANIMATION_STATE_H
#define EVIL_ANIMATION_STATE_H

#include "Animation.h"

class eAnimationController;

//******************************
//		eAnimationState
// used by eAnimationController
// as part of its state machine
// which controls which animations
// are updating an eRenderImage
//*******************************
class eAnimationState {
public:

	friend class eAnimationController;		// the only class able to call this->Update()

public:

											eAnimationState(const std::string & name, 
															const std::shared_ptr<eAnimation> & animation, 
															float speed = 1.0f);

	float									GetNormalizedTime() const;
	void									SetNormalizedTime(float normalizedTime);
	float									Duration() const;
	float									Time() const;
	const std::string &						Name() const;
	size_t									NameHash() const;
	const AnimationFrame_t &				GetCurrentFrame() const;
	void									SetAnimationController(eAnimationController * newStateMachine);

private:

	void									Update();

public:

	float									speed;

private:

	eAnimationController *					stateMachine;			// backpointer to handler, for access to the component's gameobject owner
	std::shared_ptr<eAnimation>				animation;				// which animation this state plays
	std::string								name;
	size_t									nameHash;
	float									duration;
	float									time			= 0.0f;
	AnimationFrame_t *						currentFrame	= nullptr;
};

//*********************
// eAnimationState::eAnimationState
//*********************
inline eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed)
	: name(name),
	  animation(animation),
	  speed(speed) {
	currentFrame = &animation->frames[0];
	duration = animation->duration * speed;
	nameHash = std::hash<std::string>()(name);
}

//*********************
// eAnimationState::GetNormalizedTime
// returns the fraction of its duration that this state is currently at
// range [0, 1]
//*********************
inline float eAnimationState::GetNormalizedTime() const {
	return (time / duration);
}

//*********************
// eAnimationState::SetNormalizedTime
//*********************
inline void eAnimationState::SetNormalizedTime(float normalizedTime) {
	time = normalizedTime * duration;
}

//*********************
// eAnimationState::Duration
// returns the duration of this state in milliseconds
//*********************
inline float eAnimationState::Duration() const {
	return duration;
}

//*********************
// eAnimationState::Time
// returns the un-normalized time of this state in milliseconds
// range [0, duration]
//*********************
inline float eAnimationState::Time() const {
	return time;
}

//*********************
// eAnimationState::Name
//*********************
inline const std::string & eAnimationState::Name() const {
	return name;
}

//*********************
// eAnimationState::NameHash
//*********************
inline size_t eAnimationState::NameHash() const {
	return nameHash;
}

//*********************
// eAnimationState::GetCurrentFrame
//*********************
inline const AnimationFrame_t & eAnimationState::GetCurrentFrame() const {
	return *currentFrame;
}

//*********************
// eAnimationState::SetAnimationController
//*********************
inline void eAnimationState::SetAnimationController(eAnimationController * newStateMachine) {
	stateMachine = newStateMachine;
}

#endif /* EVIL_ANIMATION_STATE_H */