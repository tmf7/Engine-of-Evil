#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

// TODO: allow for different format sprite-sheets (eg: variable frame sizes from frame to frame)
// (also see Sprite class for similar TODO)

// TODO: SDL_BlitSurface doesn't allow pixel data to be rotated, only uses AABB
// (however image rotation is implemented, the collision BBOX must also be rotated)

//***************************************
//				Image
// Provides access to pixel data as well (source)
// as a subset of that pixel data (frame)
// users must ensure frame dimensions and position
// does not exceed that of the source dimensions
//***************************************
class Image {
private:

	SDL_Surface *		source;		// pointer to the surface in the ImageManager dictionary/hashtable
	SDL_Rect			frame;

public:
						Image();
						Image(SDL_Surface * source);
						Image(SDL_Surface * source, const int frameWidth, const int frameHeight, const int frameNumber);
	
	bool				IsValid() const;
	void				SetSource(SDL_Surface * source);	
	SDL_Surface *		Source() const;
	void				SetFrame(const int frameNumber);
	SDL_Rect *			Frame();						
};

//**************
// Image::Image
//**************
inline Image::Image() : source(NULL) {
}

//**************
// Image::Image
//**************
inline Image::Image(SDL_Surface * source) : source(source) {
	frame.w = source->w;
	frame.h = source->h;
	frame.x = 0;
	frame.y = 0;
}

//**************
// Image::Image
//**************
inline Image::Image(SDL_Surface * source, const int frameWidth, const int frameHeight, const int frameNumber) : source(source) {
	frame.w = frameWidth;
	frame.h = frameHeight;
	SetFrame(frameNumber);
}

//**************
// Image::SetSource
// resets the frame size to the source's size
//**************
inline void Image::SetSource(SDL_Surface * source) {
	this->source = source;
	frame.w = source->w;
	frame.h = source->h;
	frame.x = 0;
	frame.y = 0;
}

//**************
// Image::Source
//**************
inline SDL_Surface * Image::Source() const {
	return source;
}

//**************
// Image::Frame
// direct access to frame data members x, y, width, and height
//**************
inline SDL_Rect * Image::Frame() {
	return &frame;
}

//**************
// Image::SetFrame
// select a source-size-dependent area of source
// user must ensure source data is not NULL via Image::IsValid()
//**************
inline void Image::SetFrame(const int frameNumber) {
	int sourceColumns;

	// treat the source surface as a 2D array of images
	sourceColumns = source->w / frame.w;
	frame.y = (frameNumber / sourceColumns) * frame.h;	// div gives row
	frame.x = (frameNumber % sourceColumns) * frame.w;	// mod give col
}

//**************
// Image::IsValid
// returns true if source != NULL
//**************
inline bool Image::IsValid() const {
	return source != NULL;
}
#endif /* EVIL_IMAGE_H */

