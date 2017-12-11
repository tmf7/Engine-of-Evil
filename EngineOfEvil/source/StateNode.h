/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
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

	virtual int									GetClassType() const override				{ return CLASS_STATENODE; }
	virtual bool								IsClassType(int classType) const override	{ 
													if(classType == CLASS_STATENODE) 
														return true; 
													return eClass::IsClassType(classType); 
												}

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