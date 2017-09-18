#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "SpatialIndexGrid.h"
#include "GridCell.h"

template<class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<eGridCell, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

typedef std::vector<std::pair<int,int>> pairVector_t;

typedef enum {
	TRAVERSABLE_MAP,
	COLLISION_MAP,
	RANDOM_MAP
} mapType_t;

class eMap {
public:

	bool						Init();
	bool						IsValid(const eVec2 & point, bool ignoreCollision = false) const;
	void						Think();
	void						Draw();
	bool						LoadMap(const char * mapFilename);
	void						BuildMap(const int configuration);
	void						ToggleTile(const eVec2 & point);
	tile_map_t &				TileMap();
	const pairVector_t &		VisibleCells();

private:

	tile_map_t					tileMap;
	pairVector_t				visibleCells;		// the cells currently within the camera's view
};

//**************
// eMap::TileMap
//**************
inline tile_map_t & eMap::TileMap() {
	return tileMap;
}

//**************
// eMap::VisibleCells
//**************
inline const pairVector_t & eMap::VisibleCells() {
	return visibleCells;
}

#endif /* EVIL_MAP_H */
