#ifndef EVIL_ANIMATION_STATE_H
#define EVIL_ANIMATION_STATE_H

#include "Animation.h"

typedef enum {
	ONCE,					// go to end, then stop
	REPEAT,					// go to end, the reset to beginning
//	PINGPONG				// TODO: implement go to end, then reverse to beginning, then back again
} AnimationLoopState_t;

//******************************
//		eAnimationState
// used by eAnimationController
// as part of its state machine
// which controlls which animations
// are updating an eRenderImage
//*******************************
class eAnimationState {
public:

										eAnimationState(const std::string & name, 
														const std::shared_ptr<eAnimation> & animation, 
														float speed = 1.0f, 
														AnimationLoopState_t loop = AnimationLoopState_t::ONCE);

	void								Update();
	float								GetNormalizedTime() const;
	void								SetNormalizedTime(float newNormalizedTime);
	Uint32								Duration() const;
	Uint32								Time() const;
	const std::string &					Name() const;
	size_t								NameHash() const;
	const AnimationFrame_t &			GetCurrentFrame() const;

public:

	AnimationLoopState_t				loop			= AnimationLoopState_t::ONCE;
	float								speed			= 1.0f;


private:

	std::shared_ptr<eAnimation>			animation;				// which animation this state plays
	std::string							name;
	size_t								nameHash;
	Uint32								duration;
	Uint32								time			= 0;
	float								normalizedTime	= 0.0f;
	AnimationFrame_t *					currentFrame	= nullptr;
};

//*********************
// eAnimationState::eAnimationState
//*********************
inline eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed = 1.0f, AnimationLoopState_t loop = AnimationLoopState_t::REPEAT)
	: name(name),
	  animation(animation),
	  speed(speed),
	  loop(loop) {
	currentFrame = &animation->frames[0];
	duration = (Uint32)eMath::NearestInt((float)animation->GetDuration() * speed);
	nameHash = std::hash<std::string>()(name);
}

//*********************
// eAnimationState::GetNormalizedTime
// returns the fraction of its duration that this state is currently at
// range [0, 1]
//*********************
inline float eAnimationState::GetNormalizedTime() const {
	return normalizedTime;
}

//*********************
// eAnimationState::Duration
// returns the duration of this state in milliseconds
//*********************
inline Uint32 eAnimationState::Duration() const {
	return duration;
}

//*********************
// eAnimationState::Time
// returns the un-normalized time of this state in milliseconds
// range [0, duration]
//*********************
inline Uint32 eAnimationState::Time() const {
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
	return *currentFrame; // animation->GetFrame(currentFrame);
}

#endif /* EVIL_ANIMATION_STATE_H */