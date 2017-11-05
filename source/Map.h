#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "SpatialIndexGrid.h"
#include "GridCell.h"

template<class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<eGridCell, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

typedef std::vector<std::pair<int,int>> intPairVector_t;

class eMap {
public:

	bool						Init();
	bool						HitStaticWorldHack(const eVec2 & point);
	void						Think();
	void						Draw();
	void						DebugDraw();
	bool						LoadMap(const char * mapFilename);
	void						ToggleTile(const eVec2 & point);
	eVec2						GetMouseWorldPosition() const;
	tile_map_t &				TileMap();
	const intPairVector_t &		VisibleCells();

private:

	tile_map_t					tileMap;
	intPairVector_t				visibleCells;		// the cells currently within the camera's view
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
inline const intPairVector_t & eMap::VisibleCells() {
	return visibleCells;
}

#endif /* EVIL_MAP_H */
