#ifndef EVIL_STATENODE_H
#define EVIL_STATENODE_H

#include "Animation.h"

class eAnimationController;

//******************************
//		eStateNode
// base class for states used by
// eAnimationController
// eg: eAnimationState, eStateNode
//*******************************
class eStateNode : public eClass {
public:

	friend class eAnimationController;

public:

	virtual									   ~eStateNode() = default;

	void										SetAnimationController(eAnimationController * newStateMachine);
	float										GetNormalizedTime() const;
	void										SetNormalizedTime(float normalizedTime);
	float										Duration() const;
	float										Time() const;
	const std::string &							Name() const;
	int											NameHash() const;
	const AnimationFrame_t &					GetCurrentFrame() const;

	virtual int									GetClassType() const override { return CLASS_STATENODE; }

protected:

												eStateNode() = default;

	virtual void								Update() = 0;
	void										NextFrame(const eAnimation & animation);

protected:

	eAnimationController *						stateMachine;			// back-pointer to handler, for access to the component's gameobject owner->renderImage
	std::string									name;
	int											nameHash;
	float										speed;
	float										duration;
	float										time					= 0.0f;
	const AnimationFrame_t *					currentFrame			= nullptr;
};

//*********************
// eStateNode::GetNormalizedTime
// returns the fraction of its duration that this state is currently at
// range [0, 1]
//*********************
inline float eStateNode::GetNormalizedTime() const {
	return (time / duration);
}

//*********************
// eStateNode::SetNormalizedTime
//*********************
inline void eStateNode::SetNormalizedTime(float normalizedTime) {
	time = normalizedTime * duration;
}

//*********************
// eStateNode::Duration
// returns the duration of this state in milliseconds
//*********************
inline float eStateNode::Duration() const {
	return duration;
}

//*********************
// eStateNode::Time
// returns the un-normalized time of this state in milliseconds
// range [0, duration]
//*********************
inline float eStateNode::Time() const {
	return time;
}

//*********************
// eStateNode::Name
//*********************
inline const std::string & eStateNode::Name() const {
	return name;
}

//*********************
// eStateNode::NameHash
//*********************
inline int eStateNode::NameHash() const {
	return nameHash;
}

//*********************
// eStateNode::GetCurrentFrame
//*********************
inline const AnimationFrame_t & eStateNode::GetCurrentFrame() const {
	return *currentFrame;
}

//*********************
// eStateNode::SetAnimationController
//*********************
inline void eStateNode::SetAnimationController(eAnimationController * newStateMachine) {
	stateMachine = newStateMachine;
}

#endif /* EVIL_STATENODE_H */