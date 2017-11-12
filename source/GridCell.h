#ifndef EVIL_GRIDCELL_H
#define EVIL_GRIDCELL_H

#include "Tile.h"

class eCollisionModel;

//******************************************
//			eGridCell
// container for pathfinding, collision, and tile drawing.
// DEBUG: data type used by eSpatialIndexGrid
//*****************************************
class eGridCell : public eClass {
public:
																	eGridCell() = default;

	void															Draw();
	void															DebugDraw();
	void															AddTileOwned(eTile && tile);
	const std::vector<eTile> &										TilesOwned() const;
	std::vector<eTile> &											TilesOwned();
	const std::unordered_map<eRenderImage *, eRenderImage *> &		RenderContents() const;
	std::unordered_map<eRenderImage *, eRenderImage *> &			RenderContents();
	std::unordered_map<eCollisionModel *, eCollisionModel *> &		CollisionContents();
	const eBounds &													AbsBounds() const;
	const eBounds &													AbsBounds();
	void															SetAbsBounds(const eBounds & bounds);
	void															SetGridPosition(const int row, const int column);
	int																GridRow() const;
	int																GridColumn() const;

	virtual int														GetClassType() const override { return CLASS_GRIDCELL; }

public:

	bool															inOpenSet	= false;	// expidites openSet searches
	bool															inClosedSet = false;	// expidites closedSet searches

private:

	std::unordered_map<eCollisionModel *, eCollisionModel *>		collisionContents;	// all eCollisionModel::absBounds that overlap this->absBounds
	std::unordered_map<eRenderImage *, eRenderImage *>				renderContents;		// all eRenderImage::worldClip that overlap this->absBounds
	std::vector<eTile>												tilesOwned;			// which eTiles' lifetimes are managed
	eBounds															absBounds;			// using world-coordinates
	int																gridRow;			// index within grid
	int																gridCol;			// index within grid

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
// eGridCell::AbsBounds
// DEBUG: convenience function for broad-phase collision tests
//******************
inline const eBounds & eGridCell::AbsBounds() {
	return absBounds;
}

//******************
// eGridCell::SetAbsBounds
// DEBUG: convenience function for broad-phase collision tests
//******************
inline void eGridCell::SetAbsBounds(const eBounds & bounds) {
	absBounds = bounds;
}

//******************
// eGridCell::SetGridPosition
//******************
inline void eGridCell::SetGridPosition(const int row, const int column) {
	gridRow = row;
	gridCol = column;
}

//******************
// eGridCell::GridRow
//******************
inline int eGridCell::GridRow() const {
	return gridRow;
}

//******************
// eGridCell::GridColumn
//******************
inline int eGridCell::GridColumn() const {
	return gridCol;
}

#endif /* EVIL_GRIDCELL_H */