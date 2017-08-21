#include "Tile.h"
#include "Game.h"

// FIXME: these should be class static
int									numTileTypes = 0;
std::shared_ptr<eImageTiler>		tileSet;
eTileImpl							tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::InitTileTypes
// returns false on failure to init, true otherwise
//************
bool eTileImpl::InitTileTypes(const char * tilerFilename) {
	// load the tile file
	if (!game.GetImageTilerManager().LoadTiler(tilerFilename, tileSet))
		return false;

	numTileTypes = tileSet->GetNumSequences();
	if (!numTileTypes)
		return false;

	// configure the tileTypes array according to each sequence in tileSet
	for (int i = 0; i < numTileTypes; i++) {
		std::string name = tileSet->GetSequenceName(i);
		tileTypes[i].name = name;
		tileTypes[i].type = tileSet->GetFirstIndex(name);

		if (tileTypes[i].type == invalidTileType)
			return false;

		// TODO: script this somewhere, possibly in another file type
		// that stores the .tls file to load the imageTiler
		// and then parallel data for things like collision and such
		tileTypes[i].collisionHack = tileTypes[i].type == 0 ? false : true;
	}	
	return true;
}

//************
// eTile::Init
// owner is responsible for handling all this eTile's functions (eg drawing its renderImage)
// absBounds is used for collision
// imageOffset is the tileSet image-specific offset required to position the eTile draw correctly
// type is the identifier from the tileSet eImageTiler
// layer is the draw order depth
// TODO: Initialize the collider based on procedural/file data
//************
void eTile::Init(eGridCell * owner, const eBounds & absBounds, const eVec2 & imageOffset, const int type, const int layer) {
	this->owner = owner;
	impl = &tileTypes[type];
	renderImage.image = tileSet->Source();
	renderImage.srcRect = &tileSet->GetFrame(impl->type).Frame();

	renderImage.origin = absBounds[0];
	eMath::CartesianToIsometric(renderImage.origin.x, renderImage.origin.y);
	renderImage.origin += imageOffset;
	renderImage.SetLayer(layer);

	collisionModel.SetActive(true);
	auto & localBounds = collisionModel.LocalBounds();
	eVec2 extents = eVec2(absBounds.Width() * 0.5f, absBounds.Height() * 0.5f);
	localBounds[0] = -extents;
	localBounds[1] = extents;
	collisionModel.SetOrigin(absBounds.Center());
	collisionModel.Velocity() = vec2_zero;
}