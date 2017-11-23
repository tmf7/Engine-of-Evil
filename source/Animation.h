#ifndef EVIL_ANIMATION_H
#define EVIL_ANIMATION_H

#include "Definitions.h"

typedef struct AnimationFrame_s {
	int		imageListIndex;		// within eImageManager::imageList
	int		subframeIndex;		// within eImage::subfames
	float	normalizedTime;		// range [0.0f, 1.0f]; when this frame should trigger when traversing this->frames (eg: regular update interval not mandatory) 
} AnimationFrame_t;

//*****************************
//		eAnimation
// handles designed sequences of 
// texture sub-frames
// used to produce animations
//*****************************
class eAnimation {
public:
	
										eAnimation(const std::string & name, int id, std::vector<AnimationFrame_t> & frames, int framesPerSecond);

	const AnimationFrame_t &			GetFrame(int frameIndex) const;
	int									NumFrames() const;
	int									GetFPS() const;
	void								SetFPS(int newFPS);
	Uint32								GetDuration() const;
	const std::string &					Name() const;
	size_t								NameHash() const;

private:

	std::vector<AnimationFrame_t>		frames;
	std::string							name;
	size_t								nameHash;
	Uint32								duration;
	int									framesPerSecond;
	int									animationManagerIndex;			// index within eAnimationManager::animationsList

};

//*******************
// eAnimation::eAnimation
// TODO: read an animation file, generate a vector, copy that vector, then clear it
// to start loading the next animation, if any
//*******************
inline eAnimation::eAnimation(const std::string & name, int id, std::vector<AnimationFrame_t> & frames, int framesPerSecond)
	: frames(frames),
	  framesPerSecond(framesPerSecond) {
	duration = (1000 * frames.size()) / framesPerSecond;
	nameHash = std::hash<std::string>()(name);
}

//*******************
// eAnimation::GetFrame
// TODO: use this to set the image and subframe of a renderImage in eAnimController
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
// eAnimation::GetDuration
// returns animation's duration in milliseconds
//*******************
inline Uint32 eAnimation::GetDuration() const { 
	return duration;
}

//*******************
// eAnimation::Name
//*******************
inline const std::string & eAnimation::Name() const { 
	return name; 
}

//*******************
// eAnimation::NameHash
//*******************
inline size_t eAnimation::NameHash() const { 
	return nameHash; 
}

#endif /* EVIL_ANIMATION_H */