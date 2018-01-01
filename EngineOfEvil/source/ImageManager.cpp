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
#include "Game.h"

//***************************
// eImageManager::Init
//***************************
bool eImageManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_IMAGES);

	// initialize error_image, a bright red texture the size of the current render target
	int width = game->GetRenderer().ViewArea().w;
	int height = game->GetRenderer().ViewArea().h;
	SDL_Texture * error_texture = SDL_CreateTexture(game->GetRenderer().GetSDLRenderer(),
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height);

	if (!error_texture)
		return false;

	SDL_PixelFormat * pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	Uint32 redPixel = SDL_MapRGBA(pixelFormat, 255, 0, 0, 255);
	SDL_FreeFormat(pixelFormat);

	// lock the pixels for writing
	void * pixels = nullptr;
	int pitch = 0;
	if (SDL_LockTexture(error_texture, NULL, &pixels, &pitch) == -1) {
		SDL_DestroyTexture(error_texture);
		return false;
	}

	// write the red pixels on the blank texture
	int lastPixel = pitch * height / SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888);
	for (int i = 0; i < lastPixel; i++)
		((Uint32*)pixels)[i] = redPixel;
	SDL_UnlockTexture(error_texture);

	auto & error_image = std::make_shared<eImage>(error_texture, "error_image", resourceList.size());
	std::vector<SDL_Rect> oneDefaultFrame;
	oneDefaultFrame.emplace_back(SDL_Rect{ 0, 0, error_image->GetWidth(), error_image->GetHeight() });
	error_image->SetSubframes(std::move(oneDefaultFrame));

	// register the error_image as the first element of imageList
	int hashKey = resourceHash.GetHashKey(std::string("error_image"));
	resourceHash.Add(hashKey, resourceList.size());
	resourceList.emplace_back(error_image);	// default error image
	return true;
}

//***************************
// eImageManager::LoadConstantText
// cache unchanging text images
// TODO: update to use a glyph atlas texture
// instead of loading a new texture each time
// and make it part of its own eTextAtlasManager
//***************************
bool eImageManager::LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result) {
	// image already loaded
	if ((result = GetByFilename(text))->IsValid())
		return true;

	SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, color);
	if (surfaceText == NULL) {
		result = resourceList[0]; // default error image
		return false;
	}

	SDL_Texture * renderedText = SDL_CreateTextureFromSurface(game->GetRenderer().GetSDLRenderer(), surfaceText);
	SDL_FreeSurface(surfaceText);
	if (renderedText == NULL) {
		result = resourceList[0]; // default error image
		return false;
	}

	SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);

	// register the requested text image
	int hashKey = resourceHash.GetHashKey(std::string(text));
	resourceHash.Add(hashKey, resourceList.size());
	result = std::make_shared<eImage>(renderedText, text, resourceList.size());
	resourceList.emplace_back(result);
	return true;
}

//***************************
// eImageManager::LoadSubframes
// helper function for Loading .eimg files
// see also: LoadAndGet
//***************************
bool eImageManager::LoadSubframes(std::ifstream & read, std::shared_ptr<eImage> & result) {
	int numFrames = 0;
	read >> numFrames;
	if (!VerifyRead(read))
		return false;

	static std::vector<SDL_Rect> frameList;		// static to reduce dynamic allocations
	frameList.clear();							// lazy clearing
//	frameList.reserve(numFrames);				// DEBUG: commented out to take advantage of exponential growth if needed

	// default subframe
	if (numFrames == 0) {
		read.close();
		frameList.emplace_back(SDL_Rect{ 0, 0, result->GetWidth(), result->GetHeight() });
		result->SetSubframes(std::move(frameList));
		return true;
	}

	while (!read.eof()) {
		// one subframe per line
		SDL_Rect frame;
		for (int targetData = 0; targetData < 4; targetData++) {
			switch (targetData) {
				case 0: read >> frame.x; break;
				case 1: read >> frame.y; break;
				case 2: read >> frame.w; break;
				case 3: read >> frame.h; break;
			}
		
			if (!VerifyRead(read))
				return false;
		}
		frameList.emplace_back(std::move(frame));
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	read.close();
	result->SetSubframes(frameList);
	return true;
}

//***************************
// eImageManager::LoadAndGet
// attempts to load the given .eimg file and sets result to
// either the found image and returns true, 
// or default image and returns false
// DEBUG (.eimg file format):
// textureFilepath\n
// textureAccessType numSubframes\n	
// x y w h # frame number 0 comment for reference\n
// x y w h # this is ignored 1 this is ignored\n
// x y w h # ditto 2 ditto\n
// (repeat)
// DEBUG: if numSubframes == 0, then the default subframe is the size of the image itself
// DEBUG: two .eimg files using the same texture file but different textureAccessTypes
// will generate a two eImages in eImageManager::resourceList (with unique names and values)
// textureAccessType can be:
// 0 for SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
// 1 for SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
// 2 for SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
// [NOTE]: batch image files are .bimg
//***************************
bool eImageManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eImage> & result) {
	// image already loaded
	if ((result = GetByFilename(resourceFilename))->IsValid())
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) 
		return false;

	char textureFilepath[MAX_ESTRING_LENGTH];
	memset(textureFilepath, 0, sizeof(textureFilepath));
	read.getline(textureFilepath, sizeof(textureFilepath), '\n');				// texture file
	if(!VerifyRead(read))
		return false;

	int accessInt = 0;
	SDL_TextureAccess accessType;
	read >> accessInt;															// SDL texture access type
	if (!VerifyRead(read))
		return false;

	switch(accessInt) {
		case 0: accessType = SDL_TEXTUREACCESS_STATIC; break;
		case 1: accessType = SDL_TEXTUREACCESS_STREAMING; break;
		case 2: accessType = SDL_TEXTUREACCESS_TARGET; break;
		default: accessType = SDL_TEXTUREACCESS_STATIC; break;
	}
		
	SDL_Texture * texture = NULL;
	if (accessType != SDL_TEXTUREACCESS_STATIC) {
		SDL_Surface * source = IMG_Load(textureFilepath);

		// unable to load file
		if (source == NULL) {
			result = resourceList[0]; // default error image
			return false;
		}

		texture = SDL_CreateTexture(game->GetRenderer().GetSDLRenderer(),
												  source->format->format,
												  accessType, 
												  source->w, 
												  source->h);
		// unable to initialize texture
		if (texture == NULL) {
			result = resourceList[0]; // default error image
			SDL_FreeSurface(source);
			return false;
		}
	
		// attempt to copy data to the new texture
		if (SDL_UpdateTexture(texture, NULL, source->pixels, source->pitch)) {
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(source);
			result = resourceList[0]; // default error image
			return false;
		}
		SDL_FreeSurface(source);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	} else {
		texture = IMG_LoadTexture(game->GetRenderer().GetSDLRenderer(), textureFilepath);

		// unable to initialize texture
		if (texture == NULL) {
			result = resourceList[0]; // default error image
			return false;
		}
	}

	result = std::make_shared<eImage>(texture, resourceFilename, resourceList.size());
	if (!LoadSubframes(read, result)) {											// load subframes
		result = resourceList[0];	// default error image, and destroy recently allocated result
		return false;
	}

	// register the requested image
	resourceHash.Add(result->GetNameHash(), resourceList.size());
	resourceList.emplace_back(result);
	return true;
}