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
						eImageFrame() 
							: frame(SDL_Rect{ 0, 0, 0, 0 }), 
							  next(nullptr) {
						};

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

								eImageTiler(std::shared_ptr<eImage> image, std::vector<eImageFrame> && frameList, std::vector<std::string> && sequenceNames, eHashIndex && sequenceHash, const char * filename, int id);

	std::shared_ptr<eImage>		Source() const;
	const std::string &			GetSequenceName(int sequenceNumber) const;
	int							GetFirstIndex(const std::string & sequenceName) const;
	const eImageFrame &			GetFirstFrame(const std::string & sequenceName) const;
	const eImageFrame &			GetFrame(int frameNumber) const;
	int							GetNumFrames() const;
	int							GetNumSequences() const;
	const std::string &			GetFilename() const;
	int							GetTilerManagerIndex() const;

private:

	std::vector<std::string>	sequenceNames;
	eHashIndex					sequenceHash;		// hashed-name, frameList-index pairs with index as the first of a sequence
	std::shared_ptr<eImage>		source;				// overall image to sub-divide
	std::vector<eImageFrame>	frameList;			// subsections of image to focus on
	std::string					filename;			// source file loaded from
	int							tilerManagerIndex;	// index within eImageTilerManager::tilerList
};

//**************
// eImageTiler::eImageTiler
//**************
inline eImageTiler::eImageTiler(std::shared_ptr<eImage> image, std::vector<eImageFrame> && frameList, std::vector<std::string> && sequenceNames, eHashIndex && sequenceHash, const char * filename, int id)
	: source(image),
	  frameList(std::move(frameList)),
	  sequenceNames(std::move(sequenceNames)),
	  sequenceHash(std::move(sequenceHash)),
	  filename(filename),
	  tilerManagerIndex(id) {
}

//**************
// eImageTiler::Source
//**************
inline std::shared_ptr<eImage> eImageTiler::Source() const {
	return source;
}

//**************
// eImageTiler::GetSequenceName
//**************
inline const std::string & eImageTiler::GetSequenceName(int sequenceNumber) const {
	return sequenceNames[sequenceNumber];
}


//**************
// eImageTiler::GetFirstIndex
// DEBUG: assumes each sequenceName exists,
// is is unique, and has a unique hash within this eImageTiler
// DEBUG: if any of these assumptions prove false this returns -1
//**************
inline int eImageTiler::GetFirstIndex(const std::string & sequenceName) const {
	auto hasher = std::hash<std::string>{};
	return sequenceHash.First(hasher(sequenceName));
}

//**************
// eImageTiler::GetFirstFrame
// DEBUG: assumes each sequenceName exists,
// is is unique, and has a unique hash within this eImageTiler
//**************
inline const eImageFrame & eImageTiler::GetFirstFrame(const std::string & sequenceName) const {
	auto hasher = std::hash<std::string>{};
	int index = sequenceHash.First(hasher(sequenceName));
	return frameList[index];
}

//**************
// eImageTiler::GetFrame
// get a specific frame from the imageTiler
//**************
inline const eImageFrame & eImageTiler::GetFrame(int frameNumber) const {
	return frameList[frameNumber];
}

//**************
// eImageTiler::GetNumFrames
//**************
inline int eImageTiler::GetNumFrames() const {
	return frameList.size();
}

//**************
// eImageTiler::GetNumSequences
//**************
inline int eImageTiler::GetNumSequences() const {
	return sequenceNames.size();
}

//**************
// eImageTiler::GetFilename
//**************
inline const std::string & eImageTiler::GetFilename() const {
	return filename;
}

//**************
// eImageTiler::GetTilerManagerIndex
//**************
inline int eImageTiler::GetTilerManagerIndex() const {
	return tilerManagerIndex;
}

#endif /* EVIL_IMAGE_TILER_H */