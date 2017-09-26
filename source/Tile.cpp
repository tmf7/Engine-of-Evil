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
			bool collisionHack;

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
			tileTypes[type].collisionHack = collisionHack;
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
	SetType(type);
	renderImage.SetLayer(layer);
	renderImage.origin = origin;
	eMath::CartesianToIsometric(renderImage.origin.x, renderImage.origin.y);

	// orthogonal images converted to isometric need to be shifted
	// based on their size to properly align with their owner cell
	float imageWidth = (float)renderImage.srcRect->w;
	float imageHeight = (float)renderImage.srcRect->h;
	eVec2 conversionOffset = -eVec2(imageWidth * 0.5f, (2.0f * imageHeight - imageWidth) * 0.5f);
	renderImage.origin += conversionOffset;

	// FIXME(!) not all eTile types will be solid/collidable (dont waste the processing),
	// but maintain the ability to define a collisionModel for those that are ( ie new eCollisionModel() )
	collisionModel.SetActive(true);
//	auto & localBounds = collisionModel.LocalBounds();
//	eVec2 extents = eVec2(absBounds.Width() * 0.5f, absBounds.Height() * 0.5f);
//	localBounds[0] = -extents;
//	localBounds[1] = extents;
//	collisionModel.SetOrigin(absBounds.Center());
//	collisionModel.Velocity() = vec2_zero;
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

//************
// eTile::AssignToGrid
// assign tile drawing responsibility to eGridCells visually overlapped by the renderImage.image's corners
// TODO: if the eTile::type changes, then so should the eGridCells responsible for drawing this
// because the image shape/size may change
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