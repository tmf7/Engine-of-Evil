#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "GameObject.h"

// Flyweight tile design

//***********************************************
//				eTileImpl 
// general tile type data for use by all tiles
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

	virtual int					GetClassType() const override { return CLASS_TILEIMPL; }

private:

	static const int								invalidTileType = -1;
	static const int								maxTileTypes = 256;
	static std::vector<std::pair<int, int>>			tileSet;		// first == index within eImageManager::imageList; second == eImage subframe index;
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

										eTile(eGridCell * owner, const eVec2 & origin, const int type, const int layer);
	
	int									Type() const;
	void								SetType(int newType);
	eGridCell *							GetCellOwner();

	virtual int							GetClassType() const override { return CLASS_TILE; }

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
