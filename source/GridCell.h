#ifndef EVIL_GRIDCELL_H
#define EVIL_GRIDCELL_H

#include "Tile.h"

class eCollisionModel;

//******************************************
//			eGridCell
// pathfinding, collision, and tile drawing
//***************************************
class eGridCell {
public:
										eGridCell();

	void								Draw();
	const std::vector<eTile> &			TilesOwned() const;
	std::vector<eTile> &				TilesOwned();
	const std::vector<eTile*> &			TilesToDraw() const;
	std::vector<eTile*> &				TilesToDraw();


	std::unordered_map<eCollisionModel *, eCollisionModel *> & Contents();

	const eBounds &						AbsBounds() const;
	const eBounds &						AbsBounds();
	void								SetAbsBounds(const eBounds & bounds);

private:

	std::unordered_map<eCollisionModel *, eCollisionModel *> contents;	// all colliders that overlap this

	std::vector<eTile *>				tilesToDraw;// what to draw
	std::vector<eTile>					tilesOwned;	// which eTiles' lifetimes are managed
	eBounds								absBounds;	// using world-coordinates

	// pathfinding
	eGridCell *							parent;		// originating cell to set the path back from a goal
	int									gCost;		// distance from start cell to this cell
	int									hCost;		// distance from this cell to a goal
	int									fCost;		// sum of gCost and hCost
	int									gridRow;	// index within tileMap
	int									gridCol;	// index within tileMap
	bool								inOpenSet;	// expidites PathFind openSet searches
	bool								inClosedSet;// expidites PathFind closedSet searches
};

//************
// eGridCell::eGridCell
//************
inline eGridCell::eGridCell() 
	: parent(nullptr),
	  gCost(0),
	  hCost(0),
	  fCost(0),
	  inOpenSet(false),
	  inClosedSet(false) {
}

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
inline std::vector<eTile *> & eGridCell::TilesToDraw() {
	return tilesToDraw;
}

//************
// eGridCell::TilesOwned
//************
inline const std::vector<eTile *> & eGridCell::TilesToDraw() const {
	return tilesToDraw;
}

//************
// eGridCell::Contents
//************
inline std::unordered_map<eCollisionModel *, eCollisionModel *> & eGridCell::Contents() {
	return contents;
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

#endif /* EVIL_GRIDCELL_H */