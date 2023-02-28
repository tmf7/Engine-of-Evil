/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_GRIDCELL_H
#define EVIL_GRIDCELL_H

#include "Tile.h"

class eCamera;
class eRenderTarget;
class eCollisionModel;

//******************************************
//			eGridCell
// container for pathfinding, collision, and tile drawing.
// DEBUG: data type used by eSpatialIndexGrid
//*****************************************
class eGridCell : public eGridIndex {
public:

	friend class eMap;

public:

	void															Draw(eCamera * viewCamera);
	void															DebugDraw(eRenderTarget * renderTarget);
	void															AddTileOwned(eTile && tile);
	const std::vector<eTile> &										TilesOwned() const;
	std::vector<eTile> &											TilesOwned();
	const std::unordered_map<eRenderImage *, eRenderImage *> &		RenderContents() const;
	std::unordered_map<eRenderImage *, eRenderImage *> &			RenderContents();
	std::unordered_map<eCollisionModel *, eCollisionModel *> &		CollisionContents();
	const eBounds &													AbsBounds() const;
	void															SetAbsBounds(const eBounds & bounds);
	eMap * const													GetMap();

	virtual void													Reset() override;
	virtual int														GetClassType() const override				{ return CLASS_GRIDCELL; }
	virtual bool													IsClassType(int classType) const override	{ 
																		if(classType == CLASS_GRIDCELL) 
																			return true; 
																		return eGridIndex::IsClassType(classType); 
																	}

private:

	eMap *															map;				// back-pointer to the eMap object owns the eSpatialIndexGrid than owns *this

	// FIXME (performance): make these std::vectors because most bounds sizes are small (0-30 occupied cells)
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
// eGridCell::GetMap
//************
inline eMap * const eGridCell::GetMap() {
	return map;
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
// eGridCell::RenderContents
//************
inline std::unordered_map<eRenderImage *, eRenderImage *> & eGridCell::RenderContents() {
	return renderContents;
}

//************
// eGridCell::RenderContents
//************
inline const std::unordered_map<eRenderImage *, eRenderImage *> & eGridCell::RenderContents() const {
	return renderContents;
}

//************
// eGridCell::CollisionContents
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