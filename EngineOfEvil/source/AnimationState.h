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

	virtual int								GetClassType() const override				{ return CLASS_ANIMATIONSTATE; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_ANIMATIONSTATE) 
													return true; 
												return eStateNode::IsClassType(classType); 
											}

private:

	virtual void							Update() override;

private:

	std::shared_ptr<eAnimation>				animation;				// which animation this state plays
};

#endif /* EVIL_ANIMATION_STATE_H */