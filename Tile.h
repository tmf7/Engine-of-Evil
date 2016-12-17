#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "Definitions.h"
#include "Vector.h"
#include "Image.h"
#include "Bounds.h"

// Flyweight tile design

//***********************************************
//		eTileImpl for general tile type data
//***********************************************
class eTileImpl {
private:

	friend class eTile;
	typedef void (*eTileBehavior_f)();

public:
	

						eTileImpl();
						eTileImpl(const eImage & tileImage, const int type, eTileBehavior_f behavior);

	const int			Type() const;		// what index of the tileTypes array this belongs to
	void				(*tileBehavior)();
	static bool			InitTileTypes(const char * tileSetImageFile, const char * tileFormatFile);

private:
	
	eImage				tileImage;			// all refer to same source image, type determines which frame to use
	eBounds				absCollisionBox;
	int					type;				// game-specific value to simplifiy hard-coded or scripted responses to this type

	static const int	invalidTileType = -1;
	static const int	maxTileTypes = 32;
	static eTileImpl	tileTypes[maxTileTypes];
	// TODO: other type-specific properties here (size, damage, movement?, etc)
};

//************
// eTileImpl::eTileImpl
// returns false on failure to init, true otherwise
//************
bool eTileImpl::InitTileTypes(const char * tileSetImageFile, const char * tileFormatFile) {
	// TODO:
	// read from a formatted file: image_file, type_id (which corresponds to eg eTileType::BRICK == 0)
	// then types[index++] = eTileImpl( ImageManager->Image(image_file_to_hash), type) )
/////////////////////////////////////////////////////////////
	char			buffer[MAX_STRING_LENGTH];
	char			subName[MAX_STRING_LENGTH];
	SDL_Surface *	subSurface = NULL;
	auto			hasher = std::hash<const char *>{};
	std::ifstream	in(tileFormatFile);				// the file used to breakdown the image

    if(!in.good())
        return false;

	while (1) {								// FIXME/BUG: make this stop the read/register loop at EOF

		// EG: "graphics/tiles.bmp_brick"
//		SDL_strlcpy(buffer, nullptr, 999);
		strcpy(subName, source->Name());
		strcat(subName, "_");				

		// TODO: read the name, x, y, w, h, then generate a frame for each eTile image

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
	}
    in.close();
////////////////////////////////////////////////////////////
	return false;
}

//************
// eTileImpl::eTileImpl
//************
eTileImpl::eTileImpl() : type(invalidTileType) {
}

//************
// eTileImpl::eTileImpl
//************
eTileImpl::eTileImpl(const eImage & tileImage, const int type, eTileBehavior_f behavior) 
	: tileImage(tileImage), type(type), tileBehavior(behavior) {
}

const int eTileImpl::Type() const {
	return type;
}

//***********************************************
//		eTile for localized tile data
//***********************************************
class eTile {
public:
						eTile(const eVec2 & origin, const int type);
	const int			Type() const;

private:

	eTileImpl *			impl;			// general tile type data
	eBounds				localCollisionBox;
	eVec2				origin;			// raw top-left x,y in map at large; does not account for camera position
	unsigned int		GUID;			// the unique memory index in Tile map[][]
	bool				visited;		// for fog of war queries
};

//************
// eTile::eTile
//************
inline eTile::eTile(const eVec2 & origin, const int type) 
	: origin(origin), impl(&eTileImpl::tileTypes[type]) {
}

//************
// eTile::eTile
// example use in the rest of the game:
// eTile * tile = map->Index(point);
// if (tile != nullptr && tile->Type() == eTileImpl::WATER) return false;
// OR (more flexibly) just invoke a function pointer for a tileImpl behavior
//************
inline const int eTile::Type() const {
	return impl->Type();
}

#endif /* EVIL_TILE_H */
