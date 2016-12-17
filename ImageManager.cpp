//#include <fstream>
#include <functional>
#include "ImageManager.h"
#include "Game.h"

//***************
// eImageManager::GetImage
// sets begin to a pointer to an Image if it exists
// otherwise attempts to initialize an eImage from the name (filename)
// if a format file is specified, then attempts to deconstruct the image
// into sub-images, then sets *begin and *end (contiguous) iterators accordingly
// returns false on failure at any point
// DEBUG: assert ( name != NULL )
//***************
eImage * eImageManager::GetImage(const char * name) {
	// search for pre-existing image
	auto hasher = std::hash<const char *>{};
	int hashkey = hasher(name);
	for (int i = imageHash.First(hashkey); i != -1; i = imageHash.Next(i)) {
		if (images[i].Name() && SDL_strcmp(images[i].Name(), name) == 0) {
			return &images[i];
		}
	}

	// no mathes found, initilize the image
	SDL_Surface * source = SDL_LoadBMP(name);

	// unable to initialize
	if (source == NULL)
		return nullptr;

	// attempt to format then register the requested image
	if (game.GetRenderer().FormatSurface(&source, true)) {	// FIXME: not all images should be colorKeyed
		imageHash.Add(hashkey, numImages);					// DEBUG: ++ was here originally
//		int k = imageHash.First(hashKey);
		images[numImages].Init(source, name);
		return &images[numImages++];
	} 

	// unable to configure requested image to backbuffer format
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

/*
// DEBUG: this was a trial idea prior to integrating eTiles proper
// TODO: only ever call this once per image (so either incorperate a flag in eImage, or do some other clever thing)
// return success/fail
bool eImageManager::DeconstructImage(eImage * source, const char * format, std::vector<eImage *> * result) {
	std::string		buffer;
	char			subName[MAX_STRING_LENGTH];
	SDL_Surface *	subSurface = NULL;
	auto			hasher = std::hash<const char *>{};
	std::ifstream	in(format);				// the file used to breakdown the image

    if(!in.good())
        return false;

	while (1) {								// FIXME/BUG: make this stop the read/register loop at EOF

		// EG: "graphics/tiles.bmp_brick"
		strcpy(subName, source->Name());
		strcat(subName, "_");				

		// TODO: read the name, x, y, w, h, then generate a subSurface

		// FIXME/BUG: potential issue copying pixel data from memory
		// FIXME/BUG: this also doesn't specify the SDL_Rect to use to specify the size of the new surface
//		subSurface = SDL_LoadBMP_RW((SDL_RWops *)source->Source(), 1);

		//Get the line that says 'width'
		getline(in, buffer, ' ');
		//Get the width value
		getline(in, buffer, '\n');
		width = atoi(buffer.c_str());

		//Get the line that says 'height'
		getline(in, buffer, ' ');
		//Get the width value
		getline(in, buffer, '\n');
		height = atoi(buffer.c_str());

		//Get the line that says 'tile_width'
		getline(in, buffer, ' ');
		//Get the width value
		getline(in, buffer, '\n');
		tileWidth = atoi(buffer.c_str());

		//Get the line that says 'tile_height'
		getline(in, buffer, ' ');
		//Get the width value
		getline(in, buffer, '\n');
		tileHeight = atoi(buffer.c_str());

		//Get the line taht says 'solid_tiles' and ignore it
		getline(in, buffer, '\n');
		//Get the row containing solid tiles and ignore it
		getline(in, buffer, '\n');
		//Get the row containing 'layer1' and ignore it
		getline(in, buffer, '\n');

		//Get the tile data
		data = new int[width * height];

		int i = 0;
		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
			{
				char delim = ',';

				if(x == width-1)
					delim = '\n';

				getline(in, buffer, delim);
				data[i] = atoi(buffer.c_str());
				i++;
			}
		}

		if (subSurface == NULL)
			return false;

		// attempt to format and register the sub-image
		int hashkey = hasher(subName);
		if (game.GetRenderer().FormatSurface(&subSurface, true)) {	// FIXME: not all images should be colorKeyed
			imageHash.Add(hashkey, numImages);						// DEBUG: ++ was here originally
//			int k = imageHash.First(hashKey);
			images[numImages].Init(subSurface, subName);
			result->push_back(&images[numImages]);
			numImages++;
		} else {
			return false;
		}
	}
    in.close();
	return true;
}
*/