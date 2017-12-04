#ifndef EVIL_ANIMATION_H
#define EVIL_ANIMATION_H

#include "Definitions.h"
#include "Class.h"
#include "Resource.h"

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
	
										eAnimation(const std::string & sourceFilename, int animationManagerIndex, 
												   std::vector<AnimationFrame_t> & frames,
												   int framesPerSecond,
												   AnimationLoopState loop = AnimationLoopState::ONCE);

	const AnimationFrame_t &			GetFrame(int frameIndex) const;
	int									NumFrames() const;
	int									GetFPS() const;
	void								SetFPS(int newFPS);
	float								Duration() const;

	virtual int							GetClassType() const override				{ return CLASS_ANIMATION; }
	virtual bool						IsClassType(int classType) const override	{ 
											if(classType == CLASS_ANIMATION) 
												return true; 
											return eClass::IsClassType(classType); 
										}

public:

	AnimationLoopState					loop;

private:

	std::vector<AnimationFrame_t>		frames;
	float								duration;
	int									framesPerSecond;
};

//*******************
// eAnimation::eAnimation
//*******************
inline eAnimation::eAnimation(const std::string & sourceFilename, int animationManagerIndex, std::vector<AnimationFrame_t> & frames, int framesPerSecond, AnimationLoopState loop)
	: eResource(sourceFilename, animationManagerIndex),
	  frames(frames),
	  framesPerSecond(framesPerSecond),
	  loop(loop) {
	duration = (float)(1000.0f * frames.size()) / (float)framesPerSecond;
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
// returns animation's duration in milliseconds
//*******************
inline float eAnimation::Duration() const { 
	return duration;
}

#endif /* EVIL_ANIMATION_H */