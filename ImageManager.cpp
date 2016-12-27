#include <functional>		// std::hash
#include "ImageManager.h"
#include "Game.h"

//***************
// eImageManager::GetImage
// returns a pointer to an Image if it exists
// otherwise attempts to initialize an eImage from the name (filename)
// TODO: use this to make some surfaces permanently transparent, 
// while others contextually transparent (eg: player behind a wall)
//	SDL_SetSurfaceAlphaMod(source, 64);							// apply this for uniform transparency
//	SDL_SetSurfaceColorMod(source, grey[0], grey[1], grey[2]);	// apply this to tint surfaces a given color
//	SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_BLEND);		// activates overall alpha and color blending
// DEBUG: assert ( name != NULL )
//***************
eImage * eImageManager::GetImage(const char * name, int * colorKey) {
	// search for pre-existing image
	auto hasher = std::hash<const char *>{};
	int hashkey = hasher(name);
	for (int i = imageHash.First(hashkey); i != -1; i = imageHash.Next(i)) {
		if (images[i].Name() && SDL_strcmp(images[i].Name(), name) == 0) {
			return &images[i];
		}
	}

	// no mathes found, initilize the image
	SDL_Surface * source = IMG_Load(name);

	// unable to initialize
	if (source == NULL)
		return nullptr;

	// attempt to optimize each surface for faster blit operations
	// DEBUG: this will not trigger a program failure condition if it returns -1
	// but it will result in cumulatively slower blit operations using this surface
	SDL_SetSurfaceRLE(source, 1);		

	// register the requested image
	imageHash.Add(hashkey, numImages);
	images[numImages].Init(source, name);
	return &images[numImages++];
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