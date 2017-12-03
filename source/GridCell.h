#ifndef EVIL_GRIDCELL_H
#define EVIL_GRIDCELL_H

#include "Tile.h"

class eCollisionModel;

//******************************************
//			eGridCell
// container for pathfinding, collision, and tile drawing.
// DEBUG: data type used by eSpatialIndexGrid
//*****************************************
class eGridCell : public eGridIndex {
public:

	void															Draw();
	void															DebugDraw();
	void															AddTileOwned(eTile && tile);
	const std::vector<eTile> &										TilesOwned() const;
	std::vector<eTile> &											TilesOwned();
	const std::unordered_map<eRenderImage *, eRenderImage *> &		RenderContents() const;
	std::unordered_map<eRenderImage *, eRenderImage *> &			RenderContents();
	std::unordered_map<eCollisionModel *, eCollisionModel *> &		CollisionContents();
	const eBounds &													AbsBounds() const;
	void															SetAbsBounds(const eBounds & bounds);

	virtual void													Reset() override;
	virtual int														GetClassType() const override { return CLASS_GRIDCELL; }

private:

	// FIXME (performance): make these std::vectors because most map sizes are small (0-30)
	// be sure to update eCollisionModel and eRenderImage's ::UpdateAreas fns accordingly
	std::unordered_map<eCollisionModel *, eCollisionModel *>		collisionContents;	// all eCollisionModel::absBounds that overlap this->absBounds
	std::unordered_map<eRenderImage *, eRenderImage *>				renderContents;		// all eRenderImage::worldClip that overlap this->absBounds
	std::vector<eTile>												tilesOwned;			// which eTiles' lifetimes are managed
	eBounds															absBounds;			// using world-coordinates, cached after eSpatialIndexGrid::SetCellSize to expedite collision tests

/*
	// pathfinding
	// TODO: make part of a separate A*	grid
	eGridCell *														parent		= nullptr;	// originating cell to set the path back from a goal
	int																gCost		= 0;		// distance from start cell to this cell
	int																hCost		= 0;		// distance from this cell to a goal
	int																fCost		= 0;		// sum of gCost and hCost
*/
};

//************
// eGridCell::TilesOwned
//************
inline std::vector<eTile> & eGridCell::TilesOwned() {
	return tilesOwned;
}

//************
// eGridCell::TilesOwned
//************
inline const std::vector<eTile> & eGridCell::TilesOwned() const {
	return tilesOwned;
}

//************
// eGridCell::TilesToDraw
//************
inline std::unordered_map<eRenderImage *, eRenderImage *> & eGridCell::RenderContents() {
	return renderContents;
}

//************
// eGridCell::TilesOwned
//************
inline const std::unordered_map<eRenderImage *, eRenderImage *> & eGridCell::RenderContents() const {
	return renderContents;
}

//************
// eGridCell::Contents
//************
inline std::unordered_map<eCollisionModel *, eCollisionModel *> & eGridCell::CollisionContents() {
	return collisionContents;
}

//******************
// eGridCell::AbsBounds
// DEBUG: convenience function for broad-phase collision tests
//******************
inline const eBounds & eGridCell::AbsBounds() const {
	return absBounds;
}

//******************
// eGridCell::SetAbsBounds
// DEBUG: convenience function for broad-phase collision tests
//******************
inline void eGridCell::SetAbsBounds(const eBounds & bounds) {
	absBounds = bounds;
}

#endif /* EVIL_GRIDCELL_H */