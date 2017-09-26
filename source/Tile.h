#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "CollisionModel.h"
//#include "Animation.h"
#include "Renderer.h"

// Flyweight tile design

//***********************************************
//				eTileImpl 
// general tile type data for use by all tiles
// TODO: establish better tileSet rules for 
// tileType connecting rules (pulled from a tileSet format file)
//***********************************************
class eTileImpl {
private:

	friend class eTile;
//	typedef void (*eTileBehavior_f)();

public:

	static const int		invalidTileType = -1;
	static const int		maxTileTypes = 256;

public:
							eTileImpl();

	int						Type() const;
//	void					(*tileBehavior)();

	static bool				LoadTileset(const char * tilesetFilename, bool appendNew = false);
	static int				NumTileTypes();

	static bool				IsCollidableHack(int type);
	
private:
	
	int						type;				// index within the tileSet
	bool					collisionHack;		// FIXME: temporary solution to set entire CELL of spatial index grid to TRAVERSABLE/COLLISION
};

extern std::vector<std::pair<int, int>>			tileSet;			// first == index within eImageManager::imageList; second == eImage subframe index;
extern eTileImpl								tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::NumTileTypes
//************
inline int eTileImpl::NumTileTypes() {
	return tileSet.size();
}

//************
// eTileImpl::IsCollidableHack
//************
inline bool eTileImpl::IsCollidableHack(int type) {
	return tileTypes[type].collisionHack;
}

//************
// eTileImpl::eTileImpl
//************
inline eTileImpl::eTileImpl() 
	: type(invalidTileType), 
	  collisionHack(false) {
}

//************
// eTileImpl::Type
//************
inline int eTileImpl::Type() const {
	return type;
}

//***********************************************
//				eTile 
// localized tile data
// TODO(?): have eTile inherit from eEntity to take advantage
// of the collisionModel and renderImage functionality and avoid code duplication
//***********************************************
class eTile {
public:
							eTile(eGridCell * owner, const eVec2 & origin, const int type, const int layer);
	
	int						Type() const;
	void					SetType(int newType);
	
	void					AssignToGrid();
	void					RemoveFromGrid() const;

	renderImage_t *			GetRenderImage();
	void					UpdateRenderImageDisplay();

	Uint32					GetLayer() const;
	void					SetLayer(const int newLayer);

	eGridCell *				GetOwner();
	eCollisionModel &		CollisionModel();

	bool					IsCollidableHack() const;	

private:

	eGridCell *				owner;				// responsible for drawing this tile
	eTileImpl *				impl;				// general tile type data
	eCollisionModel			collisionModel;		// contains position and size of collision bounds	// FIXME: put in eTileImpl, and just give the origin here
	renderImage_t			renderImage;		// data relevant to the renderer
};

//************
// eTile::Type
//************
inline int eTile::Type() const {
	return impl->type;
}

//************
// eTile::GetRenderImage
// FIXME: very similar to eEntity::UpdateRenderImageDisplay
//************
inline renderImage_t * eTile::GetRenderImage() {
	return &renderImage;
}

//************
// eTile::UpdateRenderImageDisplay
// FIXME: very similar to eEntity::UpdateRenderImageDisplay
// for animating sprites/tiles, use override virtual functions
// or call a virtual function within eEntity::UpdateRenderImageDispaly
// for an animator that decides what to do (or something)
// OR give the eTile a eSprite proper (...== an animator)
//************
inline void eTile::UpdateRenderImageDisplay() {
	//	eAnimationFrame * currentFrame = &tileSet->GetFirstFrame(impl->Name());
	//	currentFrame = currentFrame->Next();
}

//************
// eTile::GetLayer
//************
inline Uint32 eTile::GetLayer() const {
	return renderImage.GetLayer();
}

//************
// eTile::SetLayer
//************
inline void eTile::SetLayer(const int newLayer) {
	renderImage.SetLayer(newLayer);
}

//************
// eTile::Owner
//************
inline eGridCell * eTile::GetOwner() {
	return owner;
}

//************
// eTile::CollisionModel
//************
inline eCollisionModel & eTile::CollisionModel() {
	return collisionModel;
}

//************
// eTile::IsCollidableHack
//************
inline bool eTile::IsCollidableHack() const {
	return impl->collisionHack;
}

#endif /* EVIL_TILE_H */
