#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"
#include "Class.h"
#include "Resource.h"

//***************************************
//				eImage
// stores access pointer to SDL_Texture 
// and is handled by eImageManager
//***************************************
class eImage : public eClass , public eResource {
public:
							eImage();
							eImage(SDL_Texture * source, const char * sourceFilename, int imageManagerIndex);
	virtual					~eImage();

	SDL_Texture *			Source() const;
	int						GetWidth() const;
	int						GetHeight() const;
	
	void					SetSubframes(const std::vector<SDL_Rect> & frames);
	const SDL_Rect &		GetSubframe(int subframeIndex) const;
	int						NumSubframes() const;

	virtual int				GetClassType() const override				{ return CLASS_IMAGE; }
	virtual bool			IsClassType(int classType) const override	{ 
								if(classType == CLASS_IMAGE) 
									return true; 
								return eClass::IsClassType(classType); 
							}

private:

	std::vector<SDL_Rect>	subframes;				// sub-sections of image to focus on
	SDL_Texture *			source;
	SDL_Point				size;
};

//**************
// eImage::eImage
//**************
inline eImage::eImage()
	: eResource("invalid_file", INVALID_ID),
	  source(nullptr) {
	size = SDL_Point{ 0, 0 };
}

//**************
// eImage::eImage
// frame is the size of the texture
//**************
inline eImage::eImage(SDL_Texture * source, const char * sourceFilename, int imageManagerIndex)
	: eResource(sourceFilename, imageManagerIndex),
	  source(source) {
	SDL_QueryTexture(source, NULL, NULL, &size.x, &size.y);
}

//**************
// eImage::~eImage
//**************
inline eImage::~eImage() {
	SDL_DestroyTexture(source);
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
// eImage::SetSubframes
//**************
inline void eImage::SetSubframes(const std::vector<SDL_Rect> & frames) {
	subframes = frames;
}

//**************
// eImage::GetSubframe
// DEBUG: does no range checking
//**************
inline const SDL_Rect & eImage::GetSubframe(int subframeIndex) const {
	return subframes[subframeIndex];
}

//**************
// eImage::NumSubframes
//**************
inline int eImage::NumSubframes() const {
	return subframes.size();
}

#endif /* EVIL_IMAGE_H */

