#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "Definitions.h"
#include "Vector.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Image.h"
#include "Tile.h"

template<class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<eTile, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

typedef enum {
	TRAVERSABLE_MAP,
	COLLISION_MAP,
	RANDOM_MAP
} mapType_t;

class eMap {
public:

	bool					Init();
	bool					IsValid(const eVec2 & point, bool ignoreCollision = false) const;
	void					Think();
	void					Draw();
	void					BuildTiles(const int configuration);
	void					ToggleTile(const eVec2 & point);
	const tile_map_t &		TileMap() const;

private:

	tile_map_t				tileMap;
};

//**************
// eMap::TileMap
//**************
inline const tile_map_t & eMap::TileMap() const {
	return tileMap;
}

#endif /* EVIL_MAP_H */
