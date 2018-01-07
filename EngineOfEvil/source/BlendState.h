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
#ifndef EVIL_BLENDSTATE_H
#define EVIL_BLENDSTATE_H

#include "StateNode.h"
#include "HashIndex.h"
#include "Vector.h"

namespace evil { namespace animation {

class eAnimationController;

enum class AnimationBlendMode {
	SIMPLE_1D,
	FREEFORM_2D
//	DIRECT			// TODO: directly control node weights, and ignore squared distance auto-weights
};

//******************************
//		eBlendState
// used by eAnimationController
// as part of its state machine
// controls a vector of animations
// according to blend paramaters and mode
// see also: eAnimationState
//*******************************
class eBlendState : public eStateNode {
public:

	friend class eAnimationController;			// for direct access to Update
	friend class eAnimationControllerManager;	// for direct access to AddBlendNode


public:
												eBlendState(const std::string & name,
															int numAnimations,
															int xBlendParameterHash,
															int yBlendParameterHash,
															AnimationBlendMode blendMode = AnimationBlendMode::SIMPLE_1D,
															float speed = 1.0f);

	virtual bool								IsClassType(ClassType_t classType) const override	{ 
													if(classType == Type) 
														return true; 
													return eStateNode::IsClassType(classType); 
												}

private:

	void										Init();
	bool										AddBlendNode(const std::string & animationName, float xPosition, float yPosition = 0.0f);
	virtual void								Update() override;
	void										SwapAnimation(int animationIndex);

public:

	static ClassType_t							Type;

private:

	std::vector<std::shared_ptr<eAnimation>>	animations;				// which animations this state plays
	AnimationBlendMode							blendMode;
	int											currentAnimationIndex;

	// DEBUG: blendNodes' indexes run parallel to animations' indexes,
	// however the blendNodes are (x,y) pairs of blendParameter values
	eHashIndex									blendNodesHash;		// indexed by eAnimation::name
	std::vector<eVec2>							blendNodes;

	// eAnimationController::floatParameters of this->stateMachine to listen to for blendNodes comparison
	int											xBlendParameterHash;
	int											yBlendParameterHash;

};

REGISTER_CLASS_TYPE(eBlendState);

} }	   /* evil::animation */
#endif /* EVIL_BLENDSTATE_H */