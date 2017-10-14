#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

//***************************************
//				eImage
// stores access pointer to SDL_Texture 
// and is handled by eImageManager
//***************************************
class eImage {
public:
							eImage();
							eImage(SDL_Texture * source, const char * sourceFilename, int id);
	virtual					~eImage();

	bool					IsValid() const;
	SDL_Texture *			Source() const;
	int						GetWidth() const;
	int						GetHeight() const;
	const std::string &		GetSourceFilename() const;
	int						GetImageManagerIndex() const;
	
	void					SetSubframes(std::vector<SDL_Rect> && frames);
	const SDL_Rect &		GetSubframe(int subframeIndex) const;
	bool					HasSubframes() const;

private:

	SDL_Texture *			source;
	SDL_Point				size;
	std::string				sourceFilename;
	int						imageManagerIndex;		// index within eImageManager::imageList

	std::vector<SDL_Rect>	subframes;				// sub-sections of image to focus on, if any
};

//**************
// eImage::eImage
//**************
inline eImage::eImage()
	: source(nullptr),
	  sourceFilename("invalid_file"), 
	  imageManagerIndex(INVALID_ID) {
	size = SDL_Point{ 0, 0 };
}

//**************
// eImage::eImage
// frame is the size of the texture
//**************
inline eImage::eImage(SDL_Texture * source, const char * sourceFilename, int id)
	: source(source), 
	  sourceFilename(sourceFilename), 
	  imageManagerIndex(id) {
	SDL_QueryTexture(source, NULL, NULL, &size.x, &size.y);
}

//**************
// eImage::~eImage
//**************
inline eImage::~eImage() {
	SDL_DestroyTexture(source);
}

//**************
// eImage::IsValid
// returns true if source != NULL
//**************
inline bool eImage::IsValid() const {
	return source != nullptr;
}

//**************
// eImage::Source
//**************
inline SDL_Texture * eImage::Source() const {
	return source;
}

//**************
// eImage::GetWidth
//**************
inline int eImage::GetWidth() const {
	return size.x;
}

//**************
// eImage::GetHeight
//**************
inline int eImage::GetHeight() const {
	return size.y;
}

//**************
// eImage::GetFilename
//**************
inline std::string eImage::GetSourceFilename() const {
	return sourceFilename;
}

//**************
// eImage::GetImageManagerIndex
//**************
inline int eImage::GetImageManagerIndex() const {
	return imageManagerIndex;
}

//**************
// eImage::SetSubframes
//**************
inline void eImage::SetSubframes(std::vector<SDL_Rect> && frames) {
	subframes = std::move(frames);
}

//**************
// eImage::GetSubframe
//**************
inline const SDL_Rect & eImage::GetSubframe(int subframeIndex) const {
	return subframes.at(subframeIndex);
}

//**************
// eImage::HasSubframes
//**************
inline bool eImage::HasSubframes() const {
	return !subframes.empty();
}

#endif /* EVIL_IMAGE_H */

