#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "Definitions.h"
#include "Vector.h"
#include "Image.h"
#include "Game.h"
//#include "Bounds.h"

// Flyweight tile design

//***********************************************
//				eTileImpl 
// general tile type data for use by all tiles
// TODO: establish better tileSet rules for 
// tileType connecting rules (pulled from a tileSet format file)
//***********************************************
class eTileImpl {
private:

	friend class eTile;
//	typedef void (*eTileBehavior_f)();

public:
						eTileImpl();

	int					Type() const;		// what index of the tileTypes array this belongs to
	const char *		Name() const;
//	void				(*tileBehavior)();

	static bool			InitTileTypes(const char * tileSetImageFile, const char * tileFormatFile);

private:
	
	eImage				tileImage;			// all refer to same source image, type determines which frame to use
	int					type;				// game-specific value to simplifiy hard-coded or scripted responses to this type

	static const int	invalidTileType = -1;
	static const int	maxTileTypes = 32;
	static int			numTileTypes;
	static eTileImpl	tileTypes[maxTileTypes];
	static eImage *		tileSet;
};

int eTileImpl::numTileTypes = 0;
eImage * eTileImpl::tileSet = nullptr;

//************
// eTileImpl::eTileImpl
// returns false on failure to init, true otherwise
//************
bool eTileImpl::InitTileTypes(const char * tileSetImageFile, const char * tileFormatFile) {
	char buffer[MAX_ESTRING_LENGTH];
	char tileName[MAX_ESTRING_LENGTH];

	// load the tile file
	tileSet = game.GetImageManager().GetImage(tileSetImageFile);
	if (tileSet == nullptr)
		return false;

	std::ifstream	read(tileFormatFile);
    if(!read.good())
        return false;

	int typeIndex = 0;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the first line of the file
	while (!read.eof()) {
		tileTypes[typeIndex].type = typeIndex;

		// read and set the tile name
		// EG: "graphics/tiles.bmp_brick"
		memset(tileName, 0, sizeof(tileName));
		strcpy(tileName, tileSetImageFile);
		strcat(tileName, "_");

		// TODO(?): tileName[read.gcount()] = '\0'; 
		// TODO: Breakpoint test to ensure getline() and gcount() behave as expected
		read.getline(buffer, sizeof(buffer), ' ');
		// unrecoverable read error or improperly formatted file
		if (read.bad() || read.fail()) {
			read.clear();
			read.close();
			return false;
		}
		auto testCount = read.gcount();
		strncat(tileName, buffer, testCount);

		// initialize the image
		tileTypes[typeIndex].tileImage.Init(tileSet->Source(), tileName);
		
		// read the tile frame data
		SDL_Rect & targetFrame = tileTypes[typeIndex++].tileImage.Frame();
		for (int targetData = 0; targetData < 4; targetData++) {
			switch (targetData) {
				case 0: read >> targetFrame.x; break;
				case 1: read >> targetFrame.y; break;
				case 2: read >> targetFrame.w; break;
				case 3: read >> targetFrame.h; break;
				default: break;
			}
		
			// unrecoverable read error or improperly formatted file
			if (read.bad() || read.fail()) {
				read.clear();
				read.close();
				return false;
			}
		}
	}
	numTileTypes = typeIndex;
	read.close();
	return true;
}

//************
// eTileImpl::eTileImpl
//************
inline eTileImpl::eTileImpl() 
	: type(invalidTileType) {
}

//************
// eTileImpl::Type
//************
inline int eTileImpl::Type() const {
	return type;
}

//************
// eTileImpl::Name
// DEBUG (format): "graphics/tiles.bmp_brick"
//************
inline const char * eTileImpl::Name() const {
	return tileTypes[type].tileImage.Name();
}

//***********************************************
//		eTile for localized tile data
//***********************************************
class eTile {
public:
						eTile(const eVec2 & origin, const int type);
	int					Type() const;
	const char *		Name() const;

private:

	eTileImpl *			impl;			// general tile type data
//	eBounds				localCollisionBox;
	eVec2				origin;			// raw top-left x,y in map at large; does not account for camera position
	unsigned int		GUID;			// the unique memory index in Tile map[][]
//	bool				visited;		// for fog of war queries
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
inline int eTile::Type() const {
	return impl->Type();
}

//************
// eTile::Name
// DEBUG (format): "graphics/tiles.bmp_brick"
//************
inline const char * eTile::Name() const {
	return impl->Name();
}

#endif /* EVIL_TILE_H */
