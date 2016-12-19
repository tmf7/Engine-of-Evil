#ifndef EVIL_TILE_H
#define EVIL_TILE_H

#include "Vector.h"
#include "Image.h"

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
	static const int	maxTileTypes = 32;

public:
						eTileImpl();

	int					Type() const;		// what index of the tileTypes array this belongs to
	const char *		Name() const;
//	void				(*tileBehavior)();

	static bool			InitTileTypes(char tileSetImageFile[], char tileFormatFile[]);
	static int			NumTileTypes();

	static bool			IsCollidableHack(int type);
	int					zDepthHack() const;
	
private:
	
	eImage				tileImage;			// all refer to same source image, type determines which frame to use
	int					type;				// game-specific value to simplifiy hard-coded or scripted responses to this type
	
	int					depthHack;			// FIXME(!!!): temporary solution to assigning map depth (this should be tile specific during eMap::BuildTiles procedural generation)
	bool				collisionHack;		// FIXME: temporary solution to set entire CELL of spatial index grid to TRAVERSABLE/COLLISION
};

extern eTileImpl	tileTypes[eTileImpl::maxTileTypes];
extern eImage *		tileSet;
extern int			numTileTypes;

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
	: type(invalidTileType), depthHack(-1), collisionHack(false) {
}

//************
// eTileImpl::Type
//************
inline int eTileImpl::Type() const {
	return type;
}

//************
// eTileImpl::Name
// DEBUG (format): "graphics/tiles.bmp_brick"
//************
inline const char * eTileImpl::Name() const {
	return tileTypes[type].tileImage.Name();
}

//************
// eTileImpl::zDepthHack
//************
inline int eTileImpl::zDepthHack() const {
	return depthHack;
}

//***********************************************
//		eTile for localized tile data
//***********************************************
class eTile {
public:
						eTile();
						eTile(const eVec2 & origin, const int type);
	
	eImage *			Image();
	int					Type() const;
	void				SetType(int newType);
	const char *		Name() const;
	const eVec2 &		Origin() const;
	void				SetOrigin(const float x, const float y);
	void				SetOrigin(const eVec2 & point);

	bool				IsCollidableHack() const;
	int					zDepthHack() const;

private:

	eTileImpl *			impl;			// general tile type data
//	eBounds				localCollisionBox;
	eVec2				origin;			// raw top-left x,y in map at large; does not account for camera position
//	unsigned int		GUID;			// the unique memory index in Tile map[][]
//	bool				visited;		// for fog of war queries
};

//************
// eTile::eTile
//************
inline eTile::eTile()
	: origin(vec2_zero), impl(nullptr) {
}

//************
// eTile::eTile
//************
inline eTile::eTile(const eVec2 & origin, const int type) 
	: origin(origin), impl(&tileTypes[type]) {
}


//************
// eTile::Image
//************
inline eImage * eTile::Image() {
	return &impl->tileImage;
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
inline const char * eTile::Name() const {
	return impl->Name();
}

//************
// eTile::Origin
//************
inline const eVec2 & eTile::Origin() const {
	return origin;
}

//************
// eTile::SetOrigin
//************
inline void eTile::SetOrigin(const float x, const float y) {
	origin.Set(x, y);
}

//************
// eTile::SetOrigin
//************
inline void eTile::SetOrigin(const eVec2 & point) {
	origin = point;
}

//************
// eTile::IsCollidableHack
//************
inline bool eTile::IsCollidableHack() const {
	return impl->collisionHack;
}

//************
// eTile::zDepthHack
//************
inline int eTile::zDepthHack() const {
	return impl->depthHack;
}

#endif /* EVIL_TILE_H */
