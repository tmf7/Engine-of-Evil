#ifndef EVIL_ANIMATION_H
#define EVIL_ANIMATION_H

#include "Definitions.h"
#include "Image.h"
#include "HashIndex.h"


//***************************************
//			eAnimationFrame
// sub-section of eImage data
// for use by animations and tilesets
//***************************************
class eAnimationFrame {

	friend class eAnimation;

public:
						eAnimationFrame() 
							: frame(SDL_Rect{ 0, 0, 0, 0 }), 
							  next(nullptr) {
						};

						eAnimationFrame(const SDL_Rect & frame, eAnimationFrame * next)
							: frame(frame), 
							  next(next) {
						};

	SDL_Rect				Frame()		  { return frame; };
	const SDL_Rect &		Frame() const { return frame; };
	eAnimationFrame *		Next()	const  { return next; };

private:

	SDL_Rect				frame;
	eAnimationFrame *		next;				// next frame of an animation sequence, if any
};

//***************************************
//				eAnimation
// works with a single eImage broken into a
// collection of eAnimationFrames (clipping rectangles) and
// the animation sequences they may belong to,
// and provides direct access to SDL_Rect
// frame data by index for convenience
//***************************************
class eAnimation {
public:

									eAnimation(std::shared_ptr<eImage> image, std::vector<eAnimationFrame> && frameList, std::vector<std::string> && sequenceNames, eHashIndex && sequenceHash, const char * filename, int id);

	std::shared_ptr<eImage>			Source() const;
	const std::string &				GetSequenceName(int sequenceNumber) const;
	int								GetFirstIndex(const std::string & sequenceName) const;
	const eAnimationFrame &			GetFirstFrame(const std::string & sequenceName) const;
	const eAnimationFrame &			GetFrame(int frameNumber) const;
	int								GetNumFrames() const;
	int								GetNumSequences() const;
	const std::string &				GetFilename() const;
	int								GetAnimationManagerIndex() const;

private:

	std::vector<std::string>		sequenceNames;
	eHashIndex						sequenceHash;			// hashed-name, frameList-index pairs with index as the first of a sequence
	std::shared_ptr<eImage>			source;					// overall image to sub-divide
	std::vector<eAnimationFrame>	frameList;				// subsections of image to focus on
	std::string						filename;				// source file loaded from
	int								animationManagerIndex;	// index within eAnimationManager::animationList
};

//**************
// eAnimation::eAnimation
//**************
inline eAnimation::eAnimation(std::shared_ptr<eImage> image, std::vector<eAnimationFrame> && frameList, std::vector<std::string> && sequenceNames, eHashIndex && sequenceHash, const char * filename, int id)
	: source(image),
	  frameList(std::move(frameList)),
	  sequenceNames(std::move(sequenceNames)),
	  sequenceHash(std::move(sequenceHash)),
	  filename(filename),
	  animationManagerIndex(id) {
}

//**************
// eAnimation::Source
//**************
inline std::shared_ptr<eImage> eAnimation::Source() const {
	return source;
}

//**************
// eAnimation::GetSequenceName
//**************
inline const std::string & eAnimation::GetSequenceName(int sequenceNumber) const {
	return sequenceNames[sequenceNumber];
}


//**************
// eAnimation::GetFirstIndex
// DEBUG: assumes each sequenceName exists,
// is unique, and has a unique hash within this eAnimation
// DEBUG: if any of these assumptions prove false this returns -1
//**************
inline int eAnimation::GetFirstIndex(const std::string & sequenceName) const {
	auto hasher = std::hash<std::string>{};
	return sequenceHash.First(hasher(sequenceName));
}

//**************
// eAnimation::GetFirstFrame
// DEBUG: assumes each sequenceName exists,
// is is unique, and has a unique hash within this eAnimation
//**************
inline const eAnimationFrame & eAnimation::GetFirstFrame(const std::string & sequenceName) const {
	auto hasher = std::hash<std::string>{};
	int index = sequenceHash.First(hasher(sequenceName));
	return frameList[index];
}

//**************
// eAnimation::GetFrame
// get a specific frame from the eAnimation
//**************
inline const eAnimationFrame & eAnimation::GetFrame(int frameNumber) const {
	return frameList[frameNumber];
}

//**************
// eAnimation::GetNumFrames
//**************
inline int eAnimation::GetNumFrames() const {
	return frameList.size();
}

//**************
// eAnimation::GetNumSequences
//**************
inline int eAnimation::GetNumSequences() const {
	return sequenceNames.size();
}

//**************
// eAnimation::GetFilename
//**************
inline const std::string & eAnimation::GetFilename() const {
	return filename;
}

//**************
// eAnimation::GetAnimationManagerIndex
//**************
inline int eAnimation::GetAnimationManagerIndex() const {
	return animationManagerIndex;
}

#endif /* EVIL_ANIMATION_H */