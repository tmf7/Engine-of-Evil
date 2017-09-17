#include "Tile.h"
#include "Game.h"

// FIXME: these should be class static, not global
// first == index within eImageManager::imageList; second == eImage subframe index;
std::vector<std::pair<int, int>>		tileSet;
eTileImpl								tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::LoadTileset
// returns false on failure to load, true otherwise
// pairs image file ids and subframe indexes therein
// DEBUG (.tls file format):
// number of tiles\n
// imageFilename : subframeIndex collisionHack subframeIndex collisionHack # comment\n
// imageFilename : subframeIndex collisionHack subframeIndex collisionHack # comment\n
// (repeat)
//************
bool eTileImpl::LoadTileset(const char * tilesetFilename, bool appendNew) {
	if (!appendNew)
		tileSet.clear();

	char buffer[MAX_ESTRING_LENGTH];
	std::ifstream	read(tilesetFilename);
	// unable to find/open file
	if (!read.good())
		return false;

	// read how many tiles are about to be loaded
	// to minimize dynamic allocations
	size_t numTiles = 0;
	read >> numTiles;
	if (!VerifyRead(read))
		return false;
	tileSet.reserve(tileSet.size() + numTiles);
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	while (!read.eof()) {
		// read a source image name up to ':'
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ':');
		if(!VerifyRead(read)) 
			return false;

		// get a pointer to a source image (or try to load it if it doesn't exist yet)
		std::shared_ptr<eImage> sourceImage = nullptr;
		if (!game.GetImageManager().LoadImage(buffer, SDL_TEXTUREACCESS_STATIC, sourceImage)) 
			return false;

		int imageID = sourceImage->GetImageManagerIndex();

		// get all subframe indexes for the eImage (separated by spaces), everything after '#' is ignored
		while (read.peek() != '#') {
			int subframeIndex;
			bool collisionHack;

			read >> subframeIndex;
				if (!VerifyRead(read))
					return false;

			read >> collisionHack;
				if (!VerifyRead(read))
					return false;

			tileSet.push_back(std::pair<int, int> { imageID, subframeIndex });
			int type = tileSet.size() - 1;
			tileTypes[type].type = type;
			tileTypes[type].collisionHack = collisionHack;
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	read.close();

	if (tileSet.empty())
		return false;

	return true;
}

//************
// eTile::Init
// owner is responsible for handling all this eTile's functions (eg drawing its renderImage)
// absBounds is used for collision
// imageOffset is the tileSet image-specific offset required to position the eTile draw correctly
// type is the identifier from the tileSet eAnimation
// layer is the draw order depth
// TODO: Initialize the collider based on procedural/file data
//************
void eTile::Init(eGridCell * owner, const eBounds & absBounds, const eVec2 & imageOffset, const int type, const int layer) {
	this->owner = owner;
	impl = &tileTypes[type];
	game.GetImageManager().GetImage(tileSet.at(type).first, renderImage.image);		// which image
	renderImage.srcRect = &renderImage.image->GetSubframe(tileSet.at(type).second);	// which part of that image

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

//************
// eTile::SetType
// FIXME: doesn't verify the index
//************
void eTile::SetType(int newType) {
	impl = &tileTypes[newType];
	game.GetImageManager().GetImage(tileSet.at(newType).first, renderImage.image);		// which image
	renderImage.srcRect = &renderImage.image->GetSubframe(tileSet.at(newType).second);	// which part of that image
}