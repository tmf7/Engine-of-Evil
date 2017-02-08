#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "CollisionModel.h"
#include "ImageTiler.h"

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

	static const int	invalidTileType = -1;
	static const int	maxTileTypes = 256;

public:
						eTileImpl();

	int					Type() const;
	const std::string &	Name() const;
//	void				(*tileBehavior)();

	static bool			InitTileTypes(const char * tilerFilename);
	static int			NumTileTypes();

	static bool			IsCollidableHack(int type);
	
private:
	
	int					type;				// index of the eImageTiler tileSet used
	std::string			name;				// name of the sequence (or single tile image) in the tileSet
	bool				collisionHack;		// FIXME: temporary solution to set entire CELL of spatial index grid to TRAVERSABLE/COLLISION
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
//***********************************************
class eTile {
public:
						eTile();
						eTile(const eVec2 & origin, const int type, const int layer);
	
	const SDL_Rect &	ImageFrame() const;
	int					Type() const;
	void				SetType(int newType);
	const std::string &	Name() const;
	const eVec2 &		GetDrawOrigin() const;
	void				SetDrawOrigin(const eVec2 & point);

	Uint8				GetLayer() const;
	void				SetLayer(const int newLayer);

	eCollisionModel &	Collider();

	bool				IsCollidableHack() const;	

private:

	eTileImpl *			impl;			// general tile type data
	eCollisionModel		collider;		// contains position and size of collision bounds
	eVec2				drawOrigin;		// top-left x,y in world coordinates (does not account for camera position)
	Uint8				layer;			// layers are draw in order of lowest to highest
};


//************
// eTile::eTile
// TODO: Initialize the collider based on procedural/file data
//************
inline eTile::eTile()
	: drawOrigin(vec2_zero), 
	  impl(nullptr) {
}

//************
// eTile::eTile
// TODO: Initialize the collider based on procedural/file data
//************
inline eTile::eTile(const eVec2 & drawOrigin, const int type, const int layer)
	: drawOrigin(drawOrigin),
	  impl(&tileTypes[type]), 
	  layer(layer) {
}

//************
// eTile::ImageFrame
// TODO: currently assumes a tile is not animated
// but re-code for:
// eImageFrame result;
// tileSet->GetFirstFrame(impl->Name(), result);
// to utilize and advance an animated tile through
// currentFrame = currentFrame.Next();
//************
inline const SDL_Rect & eTile::ImageFrame() const {
	return tileSet->GetFrame(impl->type).Frame();		// FIXME(?): should this just be a wrapper to an eTileImpl call?
														// perhaps not, as each tile of a type may be at a different animation frame (maybe)
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
}

//************
// eTile::Name
// DEBUG (format): "graphics/tiles.bmp_brick"
//************
inline const std::string & eTile::Name() const {
	return impl->Name();
}

//************
// eTile::Origin
//************
inline const eVec2 & eTile::GetDrawOrigin() const {
	return drawOrigin;
}

//************
// eTile::SetOrigin
//************
inline void eTile::SetDrawOrigin(const eVec2 & point) {
	drawOrigin = point;
}

//************
// eTile::Layer
//************
inline Uint8 eTile::GetLayer() const {
	return layer;
}

//************
// eTile::SetLayer
//************
inline void eTile::SetLayer(const int newLayer) {
	layer = newLayer;
}

//************
// eTile::Collider
//************
inline eCollisionModel & eTile::Collider() {
	return collider;
}

//************
// eTile::IsCollidableHack
//************
inline bool eTile::IsCollidableHack() const {
	return impl->collisionHack;
}

#endif /* EVIL_TILE_H */
