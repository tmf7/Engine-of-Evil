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
#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "GameObject.h"

// Flyweight tile design

//***********************************************
//				eTileImpl 
// general tile type data for use by all tiles
// FIXME: make this a proper pImpl implementation (moving definitions to .cpp)
// and make tileSet a separate flyweight class, with tileTypes a pseudo-singleton object/class
//***********************************************
class eTileImpl : public eClass {
private:

//	typedef void (*eTileBehavior_f)();
	friend class				eTile;

public:
								eTileImpl();

	int							Type() const;
//	void						(*tileBehavior)();

	static bool					LoadTileset(const char * tilesetFilename, bool appendNew = false);
	static int					NumTileTypes();
	static bool					HasCollider(int type);

	virtual int					GetClassType() const override				{ return CLASS_TILEIMPL; }
	virtual bool				IsClassType(int classType) const override	{ 
									if(classType == CLASS_TILEIMPL) 
										return true; 
									return eClass::IsClassType(classType); 
								}

private:

	static const int								invalidTileType = -1;
	static const int								maxTileTypes = 1024;
	static std::vector<std::pair<int, int>>			tileSet;		// first == index within eImageManager::resourceList; second == eImage subframe index;
	static std::array<eTileImpl, maxTileTypes>		tileTypes;
	
	eVec3						renderBlockSize;		// draw order sorting
	std::shared_ptr<eBounds>	collider = nullptr;		// FIXME: make this a generic collider shape (aabb, obb, circle, line, polyline)
	int							type = invalidTileType;	// index within the tileSet
};

//************
// eTileImpl::NumTileTypes
//************
inline int eTileImpl::NumTileTypes() {
	return tileSet.size();
}

//************
// eTileImpl::HasCollider
//************
inline bool eTileImpl::HasCollider(int type) {
	return tileTypes[type].collider != nullptr;
}

//************
// eTileImpl::eTileImpl
//************
inline eTileImpl::eTileImpl() 
	: type(invalidTileType), 
	  collider(nullptr) {
}

//************
// eTileImpl::Type
//************
inline int eTileImpl::Type() const {
	return type;
}

//***********************************************
//				eTile 
// localized tile data that describes the game environment
//***********************************************
class eTile : public eGameObject {
public:

										eTile(eGridCell * owner, const eVec2 & origin, const int type, const Uint32 layer);
	
	int									Type() const;
	void								SetType(int newType);
	eGridCell *							GetCellOwner();

	virtual int							GetClassType() const override				{ return CLASS_TILE; }
	virtual bool						IsClassType(int classType) const override	{ 
											if(classType == CLASS_TILE) 
												return true; 
											return eGameObject::IsClassType(classType); 
										}

private:

	eGridCell *							cellOwner		= nullptr;		// responsible for the lifetime of *this
	eTileImpl *							impl			= nullptr;		// general tile type data
};

//************
// eTile::Type
//************
inline int eTile::Type() const {
	return impl->type;
}

//************
// eTile::GetCell
//************
inline eGridCell * eTile::GetCellOwner() {
	return cellOwner;
}

#endif /* EVIL_TILE_H */
