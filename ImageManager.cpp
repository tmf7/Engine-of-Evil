#include "ImageManager.h"
#include "Game.h"

//***************
// eImageManager::GetImage
// returns a pointer to an Image if it exists
// otherwise attempts to initialize an Image from the filename
// returns nullptr on failure
//***************
eImage * eImageManager::GetImage(const char * filename) {
	if (filename == NULL)
		return nullptr;

	// search for pre-existing image
	int key = imageHash.GenerateKey(filename);
	for (int i = imageHash.First(key); i != -1; i = imageHash.Next(i)) {
		if (images[i].Name() && SDL_strcmp(images[i].Name(), filename) == 0) {	
			return &images[i];											
		}
	}

	// no mathes found, initilize the image
	SDL_Surface * surface = SDL_LoadBMP(filename);

	// unable to initialize
	if (surface == NULL)
		return nullptr;

	if (game.GetRenderer().FormatSurface(&surface, true)) {	// FIXME: not all images should be colorKeyed
		imageHash.Add(key, numImages++);
//		int k = imageHash.First(key);
		images[numImages].Init(surface, filename);
		return &images[numImages];
	}

	// unable to format the image to the backbuffer format
	return nullptr;
}

//***************
// eImageManager::Free
// frees all resource image surfaces
//***************
void eImageManager::Free() {
	int i;

	for (i = 0; i < numImages; i++) {
		if (images[i].Source()) {
			SDL_FreeSurface(images[i].Source());
			images[i].SetSource(NULL);
		}
	}
	numImages = 0;
}
