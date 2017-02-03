#ifndef EVIL_IMAGE_TILER_H
#define EVIL_IMAGE_TILER_H

#include "Definitions.h"
#include "Image.h"
#include "HashIndex.h"


//***************************************
//				eImageFrame
// sub-section of eImage data
// for use by animations and tilesets
// has eImageTiler as a friend class
//***************************************
class eImageFrame {

	friend class eImageTiler;

public:
	
						eImageFrame(const SDL_Rect & frame, eImageFrame * next)
							: frame(frame), 
							  next(next) {
							};

	SDL_Rect			Frame()		  { return frame; };
	const SDL_Rect &	Frame() const { return frame; };
	eImageFrame *		Next()	const  { return next; };

private:

	SDL_Rect			frame;
	eImageFrame *		next;				// next frame of an animation sequence, if any
};

//***************************************
//				eImageTiler
// works with a single eImage broken into a
// collection of eImageFrames (clipping rectangles) and
// the animation sequences they may belong to,
// and provides direct access to SDL_Rect
// frame data by index for convenience
//***************************************
class eImageTiler {
public:

								eImageTiler(std::shared_ptr<eImage> image, std::vector<eImageFrame> && frameList, eHashIndex && sequenceHash, const char * filename, int id);

	std::shared_ptr<eImage>		Source() const;
	bool						GetFirstFrame(const char * sequenceName, eImageFrame & result) const;
	const SDL_Rect &			GetFrame(int frameNumber) const;
	int							GetNumFrames() const;
	const std::string &			GetFilename() const;

private:

	eHashIndex					sequenceHash;		// name, frameList-index pairs with index as the first of a sequence
	std::shared_ptr<eImage>		source;				// overall image to sub-divide
	std::vector<eImageFrame>	frameList;			// subsections of image to focus on
	std::string					filename;			// source file loaded from
	int							id;					// index within eImageTilerManager::tilerList
};

//**************
// eImageTiler::eImageTiler
//**************
inline eImageTiler::eImageTiler(std::shared_ptr<eImage> image, std::vector<eImageFrame> && frameList, eHashIndex && sequenceHash, const char * filename, int id)
	: source(image),
	  frameList(std::move(frameList)),
	  sequenceHash(std::move(sequenceHash)),
	  filename(filename),
	  id(id) {
}

//**************
// eImageTiler::Source
//**************
inline std::shared_ptr<eImage> eImageTiler::Source() const {
	return source;
}

//**************
// eImageTiler::GetFirstFrame
// DEBUG: assumes each sequenceName is unique
// and therefore has a unique hash within the
// scope of this eImageTiler
// DEBUG: duplicate sequence names are allowed
// to appear across separate eImageTilers
// returns false on failure, true otherwise
//**************
inline bool eImageTiler::GetFirstFrame(const char * sequenceName, eImageFrame & result) const {
	if (!sequenceName)
		return false;

	// check that the sequence exists
	// and that there is no name collision
	auto hasher = std::hash<std::string>{};
	int index = sequenceHash.First(hasher(sequenceName));
	if (index >= 0 && sequenceHash.Next(index) < 0) {
		result = frameList[index];
		return true;
	}
	return false;
}

//**************
// eImageTiler::GetFrame
// convenience function
//**************
inline const SDL_Rect & eImageTiler::GetFrame(int frameNumber) const {
	return frameList[frameNumber].frame;
}

//**************
// eImageTiler::GetNumFrames
//**************
inline int eImageTiler::GetNumFrames() const {
	return frameList.size();
}
//**************
// eImageTiler::GetFilename
//**************
inline const std::string & eImageTiler::GetFilename() const {
	return filename;
}

#endif /* EVIL_IMAGE_TILER_H */