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
#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "SpatialIndexGrid.h"
#include "GridCell.h"

typedef eSpatialIndexGrid<eGridCell, MAX_MAP_ROWS, MAX_MAP_COLUMNS> tile_map_t;

//*************************************************
//					eMap
// the game environment, draws as observed by an eCamera,
// owns all dynamic eEntity-type objects, and
// tracks updates to the collision-world and render-world with the
// contents of eGridCells in its eSpatialIndexGrid (eMap::tileMap)
//*************************************************
class eMap : public eClass {
public:

	bool													Init();
	void													EntityThink();
	void													Draw();
	void													DebugDraw();
	bool													LoadMap(const char * mapFilename);
	void													UnloadMap();
	tile_map_t &											TileMap();
	const tile_map_t &										TileMap() const;
	void													SetViewCamera(eCamera * newViewCamera);
	eCamera * const											GetViewCamera();

	int														AddEntity(std::unique_ptr<eEntity> && entity);
	void													RemoveEntity(int entityID);
	void													ClearAllEntities();
	std::unique_ptr<eEntity> &								GetEntity(int entityID);
	int														NumEntities() const;

	const std::vector<eGridCell *> &						VisibleCells() const;
	const std::array<std::pair<eBounds, eVec2>, 4>	&		EdgeColliders() const;
	const eBounds &											AbsBounds() const;

	virtual int												GetClassType() const override				{ return CLASS_MAP; }
	virtual bool											IsClassType(int classType) const override	{ 
																if(classType == CLASS_MAP) 
																	return true; 
																return eClass::IsClassType(classType); 
															}

private:

	void													ConfigureEntity(int newSpawnID, eEntity * entity);

private:

	eCamera *												viewCamera;			// used to clip the visibleCells before drawing to the main render target (see also eGame::renderer)
	tile_map_t												tileMap;			// owns all eTile gameObjects and tracks eRenderImages and eCollisionModels positions (ie: combined renderWorld and collisionWorld)
	std::vector<std::unique_ptr<eEntity>>					entities;			// all entities owned by *this
	std::vector<eGridCell *>								visibleCells;		// the cells currently within the camera's view
	std::array<std::pair<eBounds, eVec2>, 4>				edgeColliders;		// for collision tests against map boundaries (0: left, 1: right, 2: top, 3: bottom)
	eBounds													absBounds;			// for collision tests using AABBContainsAABB 
};

//**************
// eMap::TileMap
//**************
inline tile_map_t & eMap::TileMap() {
	return tileMap;
}

//**************
// eMap::TileMap
//**************
inline const tile_map_t & eMap::TileMap() const {
	return tileMap;
}

//**************
// eMap::VisibleCells
//**************
inline const std::vector<eGridCell *> & eMap::VisibleCells() const {
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

//**************
// eMap::SetViewCamera
//**************
inline void eMap::SetViewCamera(eCamera * newViewCamera) {
	viewCamera = newViewCamera;
}

//**************
// eMap::GetViewCamera
//**************
inline eCamera * const eMap::GetViewCamera() {
	return viewCamera;
}

#endif /* EVIL_MAP_H */
