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

	void								Draw() const;
	const std::vector<eTile> &			Tiles() const;
	std::vector<eTile> &				Tiles();
	std::vector<eCollisionModel *> &	Contents();

private:

	std::vector<eTile>					tiles;		// the tiles this cell is responsible for drawing
	std::vector<eCollisionModel *>		contents;	// colliders that overlap this eGridCell (including its eTile's colliders, and others')

	// pathfinding
	eGridCell *							parent;		// originating cell to set the path back from a goal
	int									gCost;		// distance from start cell to this cell
	int									hCost;		// distance from this cell to a goal
	int									fCost;		// sum of gCost and hCost
	int									gridRow;	// index within tileMap
	int									gridCol;	// index within tileMap
	bool								inOpenSet;	// expidites PathFind openSet searches
	bool								inClosedSet;// expidites PathFind closedSet searches

//	unsigned int						GUID;			// the unique memory index in Tile map[][] (maybe)
//	bool								visited;		// for fog of war queries
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
inline std::vector<eTile> & eGridCell::Tiles() {
	return tiles;
}

//************
// eGridCell::Tiles
//************
inline const std::vector<eTile> & eGridCell::Tiles() const {
	return tiles;
}

//************
// eGridCell::Contents
//************
inline std::vector<eCollisionModel *> & eGridCell::Contents() {
	return contents;
}

#endif /* EVIL_GRIDCELL_H */