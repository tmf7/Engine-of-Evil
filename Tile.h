#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "CollisionModel.h"
#include "ImageTiler.h"
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
	const std::string &		Name() const;
//	void					(*tileBehavior)();

	static bool				InitTileTypes(const char * tilerFilename);
	static int				NumTileTypes();

	static bool				IsCollidableHack(int type);
	
private:
	
	int						type;				// index of the eImageTiler tileSet used
	std::string				name;				// name of the sequence (or single tile image) in the tileSet
	bool					collisionHack;		// FIXME: temporary solution to set entire CELL of spatial index grid to TRAVERSABLE/COLLISION
};

extern eTileImpl					tileTypes[eTileImpl::maxTileTypes];
// typedef std::shared_ptr<eImageTiler>	TileSet_t;
// extern std::vector<TileSet_t> tileSets;
extern std::shared_ptr<eImageTiler>	tileSet;				// TODO: make this an array of tilesets (eImageTilers) to mix and match
extern int							numTileTypes;

//************
// eTileImpl::NumTileTypes
//************
inline int eTileImpl::NumTileTypes() {
	return numTileTypes;
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

//************
// eTileImpl::Name
// DEBUG (from .tls file): "water" or "grass, etc
//************
inline const std::string & eTileImpl::Name() const {
	return name;
}

//***********************************************
//				eTile 
// localized tile data
// TODO: have eTile inherit from eEntity to take advantage
// of the collisionModel and renderImage functionality and avoid code duplication
//***********************************************
class eTile {
public:
							eTile();

	void					Init(eGridCell * owner, const eBounds & absBounds, const eVec2 & imageOffset, const int type, const int layer);
	
	int						Type() const;
	void					SetType(int newType);
	const std::string &		Name() const;

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
	eCollisionModel			collisionModel;		// contains position and size of collision bounds
	renderImage_t			renderImage;		// data relevant to the renderer
};

//************
// eTile::eTile
//************
inline eTile::eTile() {
}

//************
// eTile::Type
//************
inline int eTile::Type() const {
	return impl->type;
}

//************
// eTile::SetType
//************
inline void eTile::SetType(int newType) {
	impl = &tileTypes[newType];
	renderImage.srcRect = &tileSet->GetFrame(impl->type).Frame();		// FIXME(?): eTileImpl should probably do this
}

//************
// eTile::Name
// DEBUG (format): "graphics/tiles.bmp_brick"
//************
inline const std::string & eTile::Name() const {
	return impl->Name();
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
	//	eImageFrame * currentFrame = &tileSet->GetFirstFrame(impl->Name());
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
// eTile::Onwer
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
