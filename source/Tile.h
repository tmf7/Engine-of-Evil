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
	
	void								AssignToWorldGrid();
	void								RemoveFromWorldGrid();

	Uint32								GetLayer() const;
	void								SetLayer(const int newLayer);

	eGridCell *							GetCell();
	eCollisionModel &					CollisionModel();

	virtual int							GetClassType() const override { return CLASS_TILE; }

private:

	eGridCell *							cell			= nullptr;		// responsible for the lifetime of *this
	eTileImpl *							impl			= nullptr;		// general tile type data
};

//************
// eTile::Type
//************
inline int eTile::Type() const {
	return impl->type;
}

//************
// eTile::GetLayer
// TODO(?): make this part of eGameObject
// YES, because a gameObject doesn't have to have a renderImage to be on a layer (ie just a collisionmodel for layer-specific collision)
//************
inline Uint32 eTile::GetLayer() const {
	return renderImage->GetLayer();
}

//************
// eTile::SetLayer
// TODO(?): make this part of eGameObject
//************
inline void eTile::SetLayer(const int newLayer) {
	renderImage->SetRenderBlockZFromLayer(newLayer);
}

//************
// eTile::GetCell
//************
inline eGridCell * eTile::GetCell() {
	return cell;
}

//************
// eTile::CollisionModel
//************
inline eCollisionModel & eTile::CollisionModel() {
	return *collisionModel;
}

#endif /* EVIL_TILE_H */
