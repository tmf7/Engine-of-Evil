#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "SpatialIndexGrid.h"
#include "GridCell.h"

template<class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<eGridCell, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

//*************************************************
//					eMap
// the game environment, draws as observed by eCamera,
// and handles updates to the collision-world and render-world 
// contents contained by the eGridCells in eSpatialIndexGrid (eMap::tileMap)
//*************************************************
class eMap : public eClass {
public:

	bool												Init();
	void												Draw();
	void												DebugDraw();
	bool												LoadMap(const char * mapFilename);
	tile_map_t &										TileMap();

	const std::vector<eGridCell *> &					VisibleCells();
	const std::array<std::pair<eBounds, eVec2>, 4>	&	EdgeColliders() const;
	const eBounds &										AbsBounds() const;

	virtual int											GetClassType() const override { return CLASS_MAP; }

private:

	tile_map_t											tileMap;
	std::vector<eGridCell *>							visibleCells;		// the cells currently within the camera's view
	std::array<std::pair<eBounds, eVec2>, 4>			edgeColliders;		// for collision tests against map boundaries (0: left, 1: right, 2: top, 3: bottom)
	eBounds												absBounds;			// for collision tests using AABBContainsAABB 
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
inline const std::vector<eGridCell *> & eMap::VisibleCells() {
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
