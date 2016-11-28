#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "Definitions.h"
#include "Vector.h"
#include "Image.h"

// Flyweight tile design

//***********************************************
//		eTileImpl for general tile type data
//***********************************************
class eTileImpl {
private:

	eImage * tileSet;		// all tiles refer to the same source image, tileType determines which frame of it to use
						
	const int type;			// game-specific value to simplifiy hard-coded or scripted responses to this type
	static const int INVALID_TILE_TYPE = -1;
	static const int MAX_TILE_TYPES = 32;
	
	// TODO: other type-specific properties here (size, damage, movement?, etc)
	// eVec2 bounds[2];	// mins and maxs given its general size, make this an eBounds such that it can be handled easier? for collision purposes?

public:

	// TODO: mods must update this list for new tile-based game logic to take effect
	// TODO: even better => have a map file define what the gameTiles are and forget this enum
	enum gameTiles {
		BRICK,
		WATER
	};

	eTileImpl();
	eTileImpl(eImage * tileSet, int type);
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
eTileImpl::eTileImpl() : tileSet(NULL), type(INVALID_TILE_TYPE) {
}

//************
// eTileType
//************
eTileImpl::eTileImpl(eImage * tileSet, const int type) : tileSet(tileSet), type(type) {
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
