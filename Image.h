#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

// TODO: allow for different format sprite-sheets (eg: variable frame sizes from frame to frame)
// (also see Sprite class for similar TODO)

// TODO: SDL_BlitSurface doesn't allow pixel data to be rotated, only uses AABB
// (however image rotation is implemented, the collision BBOX must also be rotated)

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

	void				Init(SDL_Surface * source, const char * filename);
	bool				IsValid() const;
	void				SetSource(SDL_Surface * source);	
	SDL_Surface *		Source() const;
	void				SetFrame(const int frameNumber);
	SDL_Rect *			Frame();
	const char *		Name() const;

private:

	SDL_Surface *		source;		// pointer to the surface in the ImageManager dictionary/hashtable
	SDL_Rect			frame;
	char				filename[MAX_FILE_PATH];
};

//**************
// eImage::eImage
//**************
inline eImage::eImage() : source(NULL) {
	memset(filename, '\0', sizeof(char) * MAX_FILE_PATH);
}

//**************
// eImage::Init
//**************
inline void eImage::Init(SDL_Surface * source, const char * filename) {
	this->source = source;
	SDL_strlcpy(this->filename, filename, MAX_FILE_PATH);		// FIXME/BUG: potential overflow
	frame.w = source->w;
	frame.h = source->h;
	frame.x = 0;
	frame.y = 0;
}

//**************
// eImage::SetSource
// resets the frame size to the source's size
//**************
inline void eImage::SetSource(SDL_Surface * source) {
	this->source = source;

	if (source == NULL)
		return;
	frame.w = source->w;
	frame.h = source->h;
	frame.x = 0;
	frame.y = 0;
}

//**************
// eImage::Source
//**************
inline SDL_Surface * eImage::Source() const {
	return source;
}

//**************
// eImage::Frame
// direct access to frame data members x, y, width, and height
//**************
inline SDL_Rect * eImage::Frame() {
	return &frame;
}

//**************
// eImage::SetFrame
// select a source-size-dependent area of source
// user must ensure source data is not NULL via Image::IsValid()
//**************
inline void eImage::SetFrame(const int frameNumber) {
	int sourceColumns;

	// treat the source surface as a 2D array of images
	sourceColumns = source->w / frame.w;
	frame.x = (frameNumber % sourceColumns) * frame.w;	// mod give col
	frame.y = (frameNumber / sourceColumns) * frame.h;	// div gives row
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
inline const char * eImage::Name() const {
	return filename;
}

#endif /* EVIL_IMAGE_H */

