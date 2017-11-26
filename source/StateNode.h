#ifndef EVIL_STATENODE_H
#define EVIL_STATENODE_H

//****************************
//		eStateNode
// base class for states used
// by eAnimationController's state machine
// eg: eAnimationState, eBlendState
//****************************
class eStateNode {
public:

	friend class eAnimationController;		// the only class able to call this->Update()

public:


	float									GetNormalizedTime() const;
	void									SetNormalizedTime(float normalizedTime);
	float									Duration() const;
	float									Time() const;
	const std::string &						Name() const;
	size_t									NameHash() const;
	const AnimationFrame_t &				GetCurrentFrame() const;
	void									SetAnimationController(eAnimationController * newStateMachine);

protected:

											eAnimationState(const std::string & name, 
															const std::shared_ptr<eAnimation> & animation, 
															float speed = 1.0f);

private:

	void									Update();

public:

	AnimationLoopState_t					loop			= AnimationLoopState_t::ONCE;		// FIXME(?): make this part of the animation instead
	float									speed			= 1.0f;


private:

	eAnimationController *					stateMachine;			// backpointer to handler, for access to the component's gameobject owner
	std::string								name;
	size_t									nameHash;
	float									duration;				// how long the current animation lasts
	float									time			= 0.0f;	// [0, duration] in increments of frameTime
	AnimationFrame_t *						currentFrame	= nullptr;
};


//*********************
// eAnimationState::eAnimationState
//*********************
inline eAnimationState::eAnimationState(const std::string & name, const std::shared_ptr<eAnimation> & animation, float speed, AnimationLoopState_t loop)
	: name(name),
	  animation(animation),
	  speed(speed),
	  loop(loop) {
	currentFrame = &animation->frames[0];
	duration = animation->duration * speed;		// FIXME: what is the duration (for calculating normalizedTime) for a vector of animations (of potentially different lengths)
	nameHash = std::hash<std::string>()(name);
}

#endif /* EVIL_STATENODE_H */