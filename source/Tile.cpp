#include "Tile.h"
#include "Game.h"

std::vector<std::pair<int, int>> eTileImpl::tileSet;		// first == index within eImageManager::imageList; second == eImage subframe index;
std::array<eTileImpl, eTileImpl::maxTileTypes> eTileImpl::tileTypes;

//************
// eTileImpl::LoadTileset
// returns false on failure to load, true otherwise
// pairs image file ids and subframe indexes therein
// DEBUG (.tls file format):
// # tileset subframes master .bsub filename comment\n
// allTilesSubframes.bsub\n
// # comment explaining default collider reference list numbering rules\n
// eBounds: width height xOffset yOffset	# 0 index comment\n
// eBounds: width height xOffset yOffset	# 1\n
// (repeat for all default colliders)
// # comment explaining default renderBlockSize reference list numbering rules\n
// rbSizeIntro: width height depth	# 0 index comment\n
// rbSizeIntro: width height depth	# 1\n
// (repeat for all default renderBlock sizes)
// # begin tile type definitions comment\n
// num_tiles: number of tiles\n
// imageFilename\n
// subframeIndex colliderType # master tile index comment\n
// (repeat tile property definitions for this file)
// # (end of tileset definition comment)\n
// imageFilename\n
// subframeIndex colliderType # master tile index comment\n
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

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// skip the first line comment
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	if (!game.GetImageManager().BatchLoadSubframes(buffer))
		return false;

	enum {
		LOADING_DEFAULT_COLLISION,
		LOADING_DEFAULT_RENDERBLOCKS,
		LOADING_TILESET
	};
	int readState = LOADING_DEFAULT_COLLISION;

// FREEHILL BEGIN AABB (eBounds) collisionModel import test (1/2)
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// skip the third line comment

	std::vector<std::shared_ptr<eBounds>> defaultAABBList;
	while (readState == LOADING_DEFAULT_COLLISION) {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ':');
		if (!VerifyRead(read))
			return false;
	
		std::string collisionShape = buffer;
		if (collisionShape == TO_STRING(eBounds)) {
			float width = 0;
			float height = 0;
			float xOffset = 0;
			float yOffset = 0;
			read >> width >> height >> xOffset >> yOffset;
			if (!VerifyRead(read))
				return false;

			std::shared_ptr<eBounds> aabb = std::make_shared<eBounds>();
			(*aabb)[1] = eVec2(width, height);
			(*aabb) += eVec2(xOffset, yOffset);
			defaultAABBList.push_back(aabb);
/*  
	TODO: implement other collision shapes and give them a common eShape/eCollider interface
	else if (collisionShape == TO_STRING(eBox)) {
	} else if (collisionShape == TO_STRING(eCircle)) {
	} else if (collisionShape == TO_STRING(eLine)) {
	} else if (collisionShape == TO_STRING(ePolyLine)) {
	}
*/
		}
		
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the rest of the line
		if (read.peek() == '#') {
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip the renderBlockSize rules comment
			readState = LOADING_DEFAULT_RENDERBLOCKS;
		}
	} 
// FREEHILL END AABB (eBounds) collisionModel import test (1/2)

// FREEHILL BEGIN renderBlockSize import (1/2)

	std::vector<eVec3> defaultRenderBlockSizes;
	while (readState == LOADING_DEFAULT_RENDERBLOCKS) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), ':');		// rbSize text
		if (!VerifyRead(read))
			return false;
	
		float width = 0;
		float height = 0;
		float depth = 0;
		read >> width >> height >> depth;
		if (!VerifyRead(read))
			return false;
	
		defaultRenderBlockSizes.emplace_back(width, height, depth); 
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the rest of the line
		if (read.peek() == '#') {
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// begin tile type definitions comment
			read.ignore(std::numeric_limits<std::streamsize>::max(), ':');	// num_tiles text
			readState = LOADING_TILESET;
		}
	} 
// FREEHILL END renderBlockSize import (1/2)

	// read how many tiles are about to be loaded
	// to minimize dynamic allocations
	size_t numTiles = 0;
	read >> numTiles;
	if (!VerifyRead(read))
		return false;

	tileSet.reserve(tileSet.size() + numTiles);
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	while (!read.eof()) {	// readState == LOADING_TILESET
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
			int colliderType;
			int renderBlockType;

			read >> subframeIndex >> colliderType >> renderBlockType;
			if (!VerifyRead(read))
				return false;

			tileSet.push_back(std::pair<int, int> { imageID, subframeIndex });		// FIXME: verify the subframe exists, 
																					// otherwise push an error image handle into this tileSet index
			int type = tileSet.size() - 1;
			tileTypes[type].type = type;
			if (colliderType > 0)
				tileTypes[type].collider = defaultAABBList[colliderType - 1];

			if (renderBlockType > 0)
				tileTypes[type].renderBlockSize = defaultRenderBlockSizes[renderBlockType - 1];

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
// layer sets *this.renderImage.renderBlock's base z-position (for draw order sorting)
// DEBUG: every tile is aligned with at least one cell no matter its size (ie: no freely-aligned tile-image origins)
//************
eTile::eTile(eGridCell * owner, const eVec2 & origin, const int type, const int layer) {
	this->owner = owner;
	renderImage.layer = layer;
	renderImage.origin = origin;
	eMath::CartesianToIsometric(renderImage.origin.x, renderImage.origin.y);
	SetType(type);
}

//************
// eTile::SetLayer
//************
void eTile::SetLayer(const int newLayer) {
	float newRBMinZ = game.GetMap().TileMap().MinLayerZ(newLayer);
	float oldRBMinZ = renderImage.renderBlock[0].z;
	renderImage.renderBlock += eVec3(0.0f, 0.0f, newRBMinZ - oldRBMinZ);
	renderImage.layer = newLayer;
}

//************
// eTile::SetType
//************
void eTile::SetType(int newType) {
	const float isoCellWidthAdjustment = (float)game.GetMap().TileMap().IsometricCellWidth() * 0.5f;
	const float isoCellHeightAdjustment = (float)game.GetMap().TileMap().IsometricCellHeight();
	float imageWidth = 0;
	float imageHeight = 0;
	eVec2 conversionOffset = vec2_zero;

	// type change, so reset the old offset and collisionModel
	if (renderImage.image != nullptr) {
		if (impl->collider != nullptr) {
			collisionModel->~eCollisionModel();
			collisionModel = nullptr;
		}
		imageWidth = (float)renderImage.srcRect->w;
		imageHeight = (float)renderImage.srcRect->h;
		conversionOffset = eVec2(isoCellWidthAdjustment, imageHeight - isoCellHeightAdjustment);
		renderImage.origin += conversionOffset;
	}

	eVec2 orthoOrigin = renderImage.origin;
	eMath::IsometricToCartesian(orthoOrigin.x, orthoOrigin.y);
	impl = &eTileImpl::tileTypes[newType];																	// FIXME(~): doesn't verify the array index
	game.GetImageManager().GetImage(eTileImpl::tileSet.at(newType).first, renderImage.image);				// which image (tile atlas)
	renderImage.srcRect = &renderImage.image->GetSubframe(eTileImpl::tileSet.at(newType).second);			// which part of that image
	renderImage.renderBlock = eBounds3D((eVec3)orthoOrigin, (eVec3)orthoOrigin + impl->renderBlockSize);	// FREEHILL 3d quicksort test
	SetLayer(renderImage.layer);

// FREEHILL BEGIN AABB (eBounds) collisionModel import test (2/2)
	if (impl->collider != nullptr) {
		collisionModel = std::make_shared<eCollisionModel>();
		collisionModel->SetActive(true);
		collisionModel->LocalBounds() = *(impl->collider);
		collisionModel->SetOrigin(orthoOrigin);
		
		// TODO: use an eTransform and offset instead of directly linking visuals and colliders
		renderImage.renderBlock += (eVec3)collisionModel->LocalBounds()[0];	// FREEHILL 3d quicksort test
	}
// FREEHILL END AABB (eBounds) collisionModel import test (2/2)

	// visual alignment with isometric owner cell
	imageWidth = (float)renderImage.srcRect->w;
	imageHeight = (float)renderImage.srcRect->h;
	conversionOffset = eVec2(-isoCellWidthAdjustment, isoCellHeightAdjustment - imageHeight);
	renderImage.origin += conversionOffset;
}

//************
// eTile::AssignToGrid
// assign tile drawing responsibility to eGridCells visually overlapped by the renderImage.image's corners
// TODO: if the eTile::type changes, then so should the eGridCells responsible for drawing *this
// DEBUG(performance): ensures no tile suddenly dissappears when scrolling the camera
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

//************
// eTile::IsCollidableHack
// FIXME: this is a hack because a eTile can have a collisionModel that lies beyond its owner cell
// and eAI currently uses point-wise tests on a cell-by-cell basis, so it ignores this case
//************
bool eTile::IsCollidableHack(const eVec2 & point) const {
	return collisionModel != nullptr ? eCollision::AABBContainsPoint(collisionModel->AbsBounds(), point) 
									 : false;

}