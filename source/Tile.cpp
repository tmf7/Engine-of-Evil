#include "Tile.h"
#include "Game.h"

// FIXME: these should be class static, not global
std::vector<std::pair<int, int>>		tileSet;		// first == index within eImageManager::imageList; second == eImage subframe index;
eTileImpl								tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::LoadTileset
// returns false on failure to load, true otherwise
// pairs image file ids and subframe indexes therein
// DEBUG (.tls file format):
// number of tiles\n
// imageFilename\n
// subframeIndex collisionHack # master tile index comment\n
// (repeat tile property definitions for this file)
// # (end of tileset definition comment)\n
// imageFilename\n
// subframeIndex collisionHack # master tile index comment\n
// # (end of tileset definition comment)\n
// (repeat image and corresponding tile definition pattern)
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
		// read a source image name
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '\n');
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
			int collisionHack;

			read >> subframeIndex;
				if (!VerifyRead(read))
					return false;

			read >> collisionHack;
				if (!VerifyRead(read))
					return false;

			tileSet.push_back(std::pair<int, int> { imageID, subframeIndex });		// FIXME: verify the subframe exists, 
																					// otherwise push an error image handle into this tileSet index
			int type = tileSet.size() - 1;
			tileTypes[type].type = type;
			tileTypes[type].collisionHack = collisionHack > 1 ? false : (bool)collisionHack;	// FIXME: double hack to test one imported AABB collisionModel

// FREEHILL BEGIN AABB (eBounds) collisionModel import test (1/2)
			if (collisionHack > 1) {
				float aabbWidth, aabbHeight, aabbXOffset, aabbYOffset;
				read >> aabbWidth;
				read >> aabbHeight;
				read >> aabbXOffset;		// TODO: not used yet (as imageOffset?)
				read >> aabbYOffset;		// TODO: not used yet (as imageOffset?)
				tileTypes[type].collisionHack2 = eBounds(vec2_zero, eVec2(aabbWidth, aabbHeight));	// generic local bounds, converted to absBounds per tile
			}
// FREEHILL END AABB (eBounds) collisionModel import test (1/2)

			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	read.close();

	if (tileSet.empty())
		return false;

	return true;
}

//************
// eTile::eTile
// owner is the originating eGridCell responsible for this eTile's lifetime
// origin is the top-left corner of the image position in world-coordinates (on an orthographic 2D grid)
// type is the index within the master tileSet vector
// layer is the draw order depth
// TODO: Initialize the collisionModel based on file data
// DEBUG: every tile is aligned with at least one cell no matter its size (ie: no freely-aligned tile-image origins)
//************
eTile::eTile(eGridCell * owner, const eVec2 & origin, const int type, const int layer) {
	this->owner = owner;
	renderImage.SetLayer(layer);
	renderImage.origin = origin;
	eMath::CartesianToIsometric(renderImage.origin.x, renderImage.origin.y);
	SetType(type, origin);
}

//************
// eTile::SetType
//************
void eTile::SetType(int newType, const eVec2 & originHack) {
	const float isoCellWidthAdjustment = (float)game.GetMap().TileMap().IsometricCellWidth() * 0.5f;
	const float isoCellHeightAdjustment = (float)game.GetMap().TileMap().IsometricCellHeight();
	float imageWidth = 0;
	float imageHeight = 0;
	eVec2 conversionOffset = vec2_zero;

	// type change, so reset the old offset
	if (renderImage.image != nullptr) {
		imageWidth = (float)renderImage.srcRect->w;
		imageHeight = (float)renderImage.srcRect->h;
		conversionOffset = eVec2(isoCellWidthAdjustment, imageHeight - isoCellHeightAdjustment);
		renderImage.origin += conversionOffset;
	}

	impl = &tileTypes[newType];															// FIXME: doesn't verify the array index
	game.GetImageManager().GetImage(tileSet.at(newType).first, renderImage.image);		// which image (tile atlas)
	renderImage.srcRect = &renderImage.image->GetSubframe(tileSet.at(newType).second);	// which part of that image

	// visual alignment with isometric owner cell
	imageWidth = (float)renderImage.srcRect->w;
	imageHeight = (float)renderImage.srcRect->h;
	conversionOffset = eVec2(-isoCellWidthAdjustment, isoCellHeightAdjustment - imageHeight);
	renderImage.origin += conversionOffset;

// FREEHILL BEGIN AABB (eBounds) collisionModel import test (2/2)
	if (impl->type == 155) {					// the test tree image
		collisionModel.SetActive(false);		// TODO: dont update grid areas yet
		collisionModel.LocalBounds() = impl->collisionHack2;
		collisionModel.SetOrigin(originHack);		// FIXME: renderImage.origin is wrong, it should be position in ortho-space, then drawn in iso-space
	}
// FREEHILL END AABB (eBounds) collisionModel import test (2/2)
}

//************
// eTile::AssignToGrid
// assign tile drawing responsibility to eGridCells visually overlapped by the renderImage.image's corners
// TODO: if the eTile::type changes, then so should the eGridCells responsible for drawing *this
// DEBUG: ensures no tile suddenly dissappears when scrolling the camera
// for a single tileMap layer this results in each eGridCell::tileToDraw::size of:
// 4 : 6 : 8, for center : edge : corner on average
// more layers increases sizes (eg: 3 layers is about 4-6 : 11 : 20, depending on map design)
//************
void eTile::AssignToGrid() {
	std::array<eVec2, 4> visualWorldPoints;
	visualWorldPoints[0] = renderImage.origin;
	visualWorldPoints[1] = renderImage.origin + eVec2((float)renderImage.srcRect->w, 0.0f);
	visualWorldPoints[2] = renderImage.origin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h);
	visualWorldPoints[3] = renderImage.origin + eVec2(0.0f, (float)renderImage.srcRect->h);

	// clip rectangle to orthographic world-space for proper grid alignment
	auto & tileMap = game.GetMap().TileMap();
	int row;
	int column;
	for (auto & point : visualWorldPoints) {
		eMath::IsometricToCartesian(point.x, point.y);
		auto & cell = tileMap.IndexValidated(point);
		auto & searchTiles = cell.TilesToDraw();
		if (std::find(searchTiles.begin(), searchTiles.end(), this) == searchTiles.end())
			searchTiles.push_back(this);
	}
}

//************
// eTile::RemoveFromGrid
// remove all responsibility of drawing this tile from the tileMap
//************
void eTile::RemoveFromGrid() const {
	// TODO: implement because if a tileType changes different eGridCells may draw it
	// otherwise loading a new map just clears the tileMap anyway without using this fn
}