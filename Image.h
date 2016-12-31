#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

//***************************************
//				eImage
// Provides access to pixel data as well (source)
// as a subset of that pixel data (frame)
// users must ensure frame dimensions and position
// does not exceed that of the source dimensions
//***************************************
class eImage {
public:

							eImage();

	void					Init(SDL_Texture * source, const char * name);
	bool					IsValid() const;
	void					SetSource(SDL_Texture * source);
	SDL_Texture *			Source() const;
	void					SetSubImage(const int frameNumber);	// FIXME: heavily modify this logic**********
	SDL_Rect &				Frame();
	const SDL_Rect &		Frame() const;
	std::string				Name() const;

private:

	SDL_Texture *			source;
	std::vector<SDL_Rect>	subFrames;			// list of all SDL_Texture sub-image frames
	SDL_Rect *				focusFrame;			// currently used sub-image frame
	std::string				name;
};

//**************
// eImage::eImage
//**************
inline eImage::eImage() : source(nullptr) {
}

//**************
// eImage::Init
// current frame is left undefined
//**************
inline void eImage::Init(SDL_Texture * source, const char * name) {
	this->source = source;
	this->name = name;
	focusFrame = nullptr;
}

//**************
// eImage::SetSource
// invalidates the current frame
//**************
inline void eImage::SetSource(SDL_Texture * source) {
	this->source = source;
	focusFrame = nullptr;
}

//**************
// eImage::Source
//**************
inline SDL_Texture * eImage::Source() const {
	return source;
}

//**************
// eImage::Frame
// mutable access to frame data members x, y, width, and height
//**************
inline SDL_Rect & eImage::Frame() {
	return *focusFrame;
}

//**************
// eImage::Frame
// read-only access to frame data members x, y, width, and height
//**************
inline const SDL_Rect & eImage::Frame() const {
	return *focusFrame;
}

//**************
// eImage::SetSubImage
// select a source-size-dependent area of source
// user must ensure source data is not NULL via Image::IsValid()
// DEBUG: this indirectly modifies frame data
// FIXME/BUG: alter this logic heavily*************************
//**************
inline void eImage::SetSubImage(const int frameNumber) {
	int sourceColumns;
	int textureWidth, textureHeight;

	// treat the source surface as a 2D array of images
	// FIXME/BUG: focusFrame is potentially nullptr (constructed, init, and set all leave it nullptr)
	SDL_QueryTexture(source, NULL, NULL, &textureWidth, &textureHeight);
	sourceColumns = textureWidth / focusFrame->w;
	focusFrame->x = (frameNumber % sourceColumns) * focusFrame->w;	// mod give col
	focusFrame->y = (frameNumber / sourceColumns) * focusFrame->h;	// div gives row
}

//**************
// eImage::IsValid
// returns true if source != NULL
//**************
inline bool eImage::IsValid() const {
	return source != NULL;
}

//**************
// eImage::Name
//**************
inline std::string eImage::Name() const {
	return name;
}

#endif /* EVIL_IMAGE_H */

