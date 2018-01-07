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
#ifndef EVIL_ANIMATION_H
#define EVIL_ANIMATION_H

#include "Definitions.h"
#include "Class.h"
#include "Resource.h"

namespace evil { namespace animation {

typedef struct AnimationFrame_s {
	int		imageManagerIndex	= 0;	// within eImageManager::resourceList
	int		subframeIndex		= 0;	// within eImage::subfames
	float	normalizedTime		= 0.0f;	// range [0.0f, 1.0f]; when this frame should trigger when traversing this->frames (eg: regular update interval not mandatory) 
} AnimationFrame_t;

enum class AnimationLoopState {
	ONCE,								// go to end, then stop
	REPEAT								// go to end, then reset to beginning
//	PINGPONG							// TODO: implement go to end, then reverse to beginning, then back again
};

//*****************************
//		eAnimation
// handles designed sequences of 
// texture sub-frames
// used to produce animations
//*****************************
class eAnimation : public eClass, public eResource {
public:

	friend class eStateNode;

public:
	
										eAnimation(const char * sourceFilename, int animationManagerIndex, 
												   std::vector<AnimationFrame_t> & frames,
												   int framesPerSecond,
												   AnimationLoopState loop = AnimationLoopState::ONCE);

	const AnimationFrame_t &			GetFrame(int frameIndex) const;
	int									NumFrames() const;
	int									GetFPS() const;
	void								SetFPS(int newFPS);
	float								Duration() const;

	virtual bool						IsClassType(ClassType_t classType) const override	{ 
											if(classType == Type) 
												return true; 
											return eClass::IsClassType(classType); 
										}

public:

	static ClassType_t					Type;

	AnimationLoopState					loop;

private:

	std::vector<AnimationFrame_t>		frames;
	float								duration;
	int									framesPerSecond;
};

REGISTER_CLASS_TYPE(eAnimation);

//*******************
// eAnimation::eAnimation
//*******************
inline eAnimation::eAnimation(const char * sourceFilename, int animationManagerIndex, std::vector<AnimationFrame_t> & frames, int framesPerSecond, AnimationLoopState loop)
	: eResource(sourceFilename, animationManagerIndex),
	  frames(frames),
	  framesPerSecond(framesPerSecond),
	  loop(loop) {
	duration = (float)frames.size() / (float)framesPerSecond;
}

//*******************
// eAnimation::GetFrame
//*******************
inline const AnimationFrame_t & eAnimation::GetFrame(int frameIndex) const {
	return frames[frameIndex];
}

//*******************
// eAnimation::NumFrames
//*******************
inline int eAnimation::NumFrames() const { 
	return frames.size(); 
}

//*******************
// eAnimation::GetFPS
//*******************
inline int eAnimation::GetFPS() const {
	return framesPerSecond; 
}

//*******************
// eAnimation::SetFPS
//*******************
inline void eAnimation::SetFPS(int newFPS) { 
	framesPerSecond = newFPS; 
}

//*******************
// eAnimation::Duration
// returns animation's duration in seconds
//*******************
inline float eAnimation::Duration() const { 
	return duration;
}

} }	   /* evil::animation */
#endif /* EVIL_ANIMATION_H */