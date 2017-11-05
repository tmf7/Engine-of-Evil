#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "SpatialIndexGrid.h"
#include "GridCell.h"

template<class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<eGridCell, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

class eMap {
public:

	bool						Init();
	void						Think();
	void						Draw();
	void						DebugDraw();
	bool						LoadMap(const char * mapFilename);
	void						ToggleTile(const eVec2 & point);
	eVec2						GetMouseWorldPosition() const;
	tile_map_t &				TileMap();

	const std::vector<std::pair<int, int>> &			VisibleCells();
	const std::array<std::pair<eBounds, eVec2>, 4>	&	EdgeColliders() const;
	const eBounds &										AbsBounds() const;

private:

	tile_map_t									tileMap;
	std::vector<std::pair<int, int>>			visibleCells;		// the cells currently within the camera's view
	std::array<std::pair<eBounds, eVec2>, 4>	edgeColliders;		// for collision tests against map boundaries (0: left, 1: right, 2: top, 3: bottom)
	eBounds										absBounds;			// for collision tests using AABBContainsAABB 
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
inline const std::vector<std::pair<int, int>> & eMap::VisibleCells() {
	return visibleCells;
}

//**************
// eMap::EdgeColliders
//**************
inline const std::array<std::pair<eBounds, eVec2>, 4> & eMap::EdgeColliders() const {
	return edgeColliders;
}

//**************
// eMap::AbsBounds
//**************
inline const eBounds & eMap::AbsBounds() const {
	return absBounds;
}

#endif /* EVIL_MAP_H */
