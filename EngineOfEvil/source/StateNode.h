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

namespace evil {

class eAnimationController;

//******************************
//		eStateNode
// base class for states used by
// eAnimationController
// eg: eAnimationState, eStateNode
//*******************************
class eStateNode : public eClass {

	ECLASS_DECLARATION(eStateNode)

public:

	friend class eAnimationController;

public:

	virtual									   ~eStateNode() = default;

												// fraction of duration that this state is currently at, range [0, 1]
	float										GetNormalizedTime() const											{ return (time / duration); }
	void										SetNormalizedTime(float normalizedTime)								{ time = normalizedTime * duration; }
	float										Duration() const													{ return duration; }
	float										Time() const														{ return time; }
	const std::string &							Name() const														{ return name; }
	int											NameHash() const													{ return nameHash; }
	const AnimationFrame_t &					GetCurrentFrame() const												{ return *currentFrame; }
	void										SetAnimationController(eAnimationController * newStateMachine)		{ stateMachine = newStateMachine; }

protected:

												eStateNode() = default;

	virtual void								Update() = 0;
	void										NextFrame(const eAnimation & animation);

protected:

	eAnimationController *						stateMachine;						// back-pointer to handler, for access to the component's gameobject owner->renderImage
	std::string									name;								// user-defined label
	int											nameHash;
	float										speed;								// non-destructively scales duration of this sstate
	float										duration;							// duration of this state in seconds
	float										time					= 0.0f;		// un-normalized time of this state in seconds, range [0, duration]
	const AnimationFrame_t *					currentFrame			= nullptr;
};

}      /* evil */
#endif /* EVIL_STATENODE_H */