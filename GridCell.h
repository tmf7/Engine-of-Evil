#ifndef EVIL_GRIDCELL_H
#define EVIL_GRIDCELL_H

#include "Tile.h"

class eCollisionModel;

//******************************************
//			eGridCell
// pathfinding, collision, and tile drawing
//***************************************
class eGridCell {
private:

	static constexpr const int			maxTiles = 1;

public:
										eGridCell();

	void								Draw();
	const std::array<eTile, maxTiles> &	Tiles() const;
	std::array<eTile, maxTiles> &		Tiles();
	std::vector<eCollisionModel *> &	Contents();

	const eBounds &						AbsBounds() const;
	const eBounds &						AbsBounds();
	void								SetAbsBounds(const eBounds & bounds);

private:

	std::array<eTile, maxTiles>			tiles;		// the tiles this cell is responsible for drawing
	std::vector<eCollisionModel *>		contents;	// colliders that overlap this eGridCell (including its eTile's colliders, and others')

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
// eGridCell::Tiles
//************
inline std::array<eTile, eGridCell::maxTiles> & eGridCell::Tiles() {
	return tiles;
}

//************
// eGridCell::Tiles
//************
inline const std::array<eTile, eGridCell::maxTiles> & eGridCell::Tiles() const {
	return tiles;
}

//************
// eGridCell::Contents
//************
inline std::vector<eCollisionModel *> & eGridCell::Contents() {
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