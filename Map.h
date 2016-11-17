#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "Definitions.h"
#include "Vector.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Image.h"

typedef enum {
	TRAVERSABLE_TILE,
	COLLISION_TILE,
	RANDOM_TILE
} tileType_t;

class eMap {
public:

							eMap();

	bool					Init();
	bool					IsValid(const eVec2 & point, bool ignoreCollision = false) const;
	void					Think();
	void					Draw() const;
	void					BuildTiles(const tileType_t type);
	void					ToggleTile(const eVec2 & point);
	const byte_map_t &		TileMap() const;

private:

	eImage *				tileSet;
	byte_map_t				tileMap;
};

//**************
// eMap::eMap
//**************
inline eMap::eMap() {
}

//**************
// eMap::TileMap
//**************
inline const byte_map_t & eMap::TileMap() const {
	return tileMap;
}

#endif /* EVIL_MAP_H */
