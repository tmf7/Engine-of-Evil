/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
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

