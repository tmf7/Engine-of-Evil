#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "Game.h"

// Flyweight tile design

//***********************************************
//		eTileImpl for general tile type data
//***********************************************
class eTileImpl {
private:

	SDL_Surface * image;	// pointer into the ImageManager's surface array
						
							// TODO:
							// ImageManager initializes surfaces as requested by objects and returns a POINTER to the surface
							// each ImageManager image gets a unique ID (hashtable?) (based on its file? or turn order)
							// such that if a requested file is already loaded, then ImageManager just returns the pointer
							// without setting surface data
							// if an object that uses a surface pointer goes out of scope, its pointer gets deleted
							// but the surface data remains
							// WHICH leaves it up to the ImageManager to free all the surfaces at shutdown
				


	const int type;			// game-specific value to simplifiy hard-coded or scripted responses to this type
	static const int INVALID_TILE_TYPE = -1;
	static const int MAX_TILE_TYPES = 32;
	
	// TODO: other type-specific properties here (size, damage, movement?, etc)
	// eVec2 bounds[2];	// mins and maxs given its general size, make this an eBounds such that it can be handled easier?

public:

	// TODO: mods must update this list for new tile-based game logic to take effect
	enum gameTiles {
		BRICK,
		WATER
	};

	eTileImpl();
	eTileImpl(SDL_Surface * image, int type);
	const int Type() const;
	static bool InitTileTypes();
	static eTileImpl tileTypes[MAX_TILE_TYPES];
	// other methods here
};

// returns false on failure to init, true otherwise
bool eTileImpl::InitTileTypes() {
	// TODO:
	// read from a formatted file: image_file, type_id (which corresponds to eg eTileType::BRICK == 0)
	// then types[index++] = eTileImpl( ImageManager->Image(image_file_to_hash), type) )
	return false;
}

//************
// eTileType
//************
eTileImpl::eTileImpl() : image(NULL), type(INVALID_TILE_TYPE) {
}

//************
// eTileType
//************
eTileImpl::eTileImpl(SDL_Surface * image, const int type) : image(image), type(type) {
	// TODO: other properties?	 
}

const int eTileImpl::Type() const {
	return type;
}

//***********************************************
//		eTile for localized tile data
//***********************************************
class eTile {
public:
	eTile(eVec2 position, int type);
	const int Type() const;

private:
	eTileImpl * impl;	// general tile type data

	eVec2 position;		// raw top-left x,y in map at large; does not account for camera position
	unsigned int ID;	// the unique memory index in Tile map[][]
	bool visited;		// for fog of war queries
	// TODO: generate a linked list of neighbors/entities for collision testing / ray tracing?
};

// FIXME: the type is not the ID, the type should be the eTileType pointer
eTile::eTile(eVec2 position, int type) : position(position) {
	impl = &eTileImpl::tileTypes[type];
	//other data?
}

// example use in the rest of the game:
// eTile * tile = map->Index(point);
// if (tile != nullptr && tile->Type() == eTileImpl::WATER) return false;
const int eTile::Type() const {
	return impl->Type();
}

#endif /* EVIL_TILE_H */
