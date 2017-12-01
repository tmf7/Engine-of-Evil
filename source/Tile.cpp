#include "Game.h"

std::vector<std::pair<int, int>> eTileImpl::tileSet;		// first == index within eImageManager::resourceList; second == eImage subframe index;
std::array<eTileImpl, eTileImpl::maxTileTypes> eTileImpl::tileTypes;

//************
// eTileImpl::LoadTileset
// returns false on failure to load, true otherwise
// pairs image file ids and subframe indexes therein
// DEBUG (.etls file format):
// # tileset image master .bimg filename comment\n
// allTileImages.bimg\n
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
// imageFilename.eimg\n
// subframeIndex colliderType # master tile index comment\n
// (repeat tile property definitions for this file)
// # (end of tileset definition comment)\n
// imageFilename.eimg\n
// subframeIndex colliderType # master tile index comment\n
// # (end of tileset definition comment)\n
// (repeat image and corresponding tile definition pattern)
//************
bool eTileImpl::LoadTileset(const char * tilesetFilename, bool appendNew) {
	if (!appendNew)
		tileSet.clear();

	std::ifstream	read(tilesetFilename);
	// unable to find/open file
	if (!read.good())
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// skip the first line comment
	char buffer[MAX_ESTRING_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	if (!game.GetImageManager().BatchLoad(buffer))
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
			defaultAABBList.emplace_back(aabb);
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

		// get a pointer to a source image
		auto & sourceImage = game.GetImageManager().Get(buffer);
		if (!sourceImage->IsValid())
			return false;

		int imageID = sourceImage->GetManagerIndex();

		// get all subframe indexes for the eImage (separated by spaces), everything after '#' is ignored
		while (read.peek() != '#') {
			int subframeIndex;
			int colliderType;
			int renderBlockType;

			read >> subframeIndex >> colliderType >> renderBlockType;
			if (!VerifyRead(read))
				return false;

			tileSet.emplace_back(std::pair<int, int> { imageID, subframeIndex });		// FIXME: verify the subframe exists, 
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
// layer sets *this.renderImage.renderBlock's base z-position (for draw-order sorting)
// DEBUG: every tile is aligned with at least one cell no matter its size (ie: no freely-aligned tile-image origins)
//************
eTile::eTile(eGridCell * cellOwner, const eVec2 & origin, const int type, const int layer) 
	: cellOwner(cellOwner) {
	orthoOrigin = origin;
	worldLayer = layer;
	renderImage = std::make_unique<eRenderImage>(this);		// all tiles currently have a renderimage by default
	SetType(type);
}

//************
// eTile::SetType
//************
void eTile::SetType(int newType) {
	// type change, so reset the collisionModel, if any
	if (renderImage->GetImage() != nullptr && impl->collider != nullptr)
		collisionModel = nullptr;

	impl = &eTileImpl::tileTypes[newType];																	// FIXME(~): doesn't verify the array index
	renderImage->Image() = game.GetImageManager().Get(eTileImpl::tileSet.at(newType).first);				// which image (tile atlas)
	renderImage->SetImageFrame(eTileImpl::tileSet.at(newType).second);										// which part of that image

	renderImage->RenderBlock() = eBounds3D((eVec3)orthoOrigin, (eVec3)orthoOrigin + impl->renderBlockSize);	// FREEHILL 3d topological sort
	renderImage->SnapRenderBlockToLayer();

// FREEHILL BEGIN AABB (eBounds) collisionModel import test (2/2)
	if (impl->collider != nullptr) {
		collisionModel = std::make_unique<eCollisionModel>(this);
		collisionModel->SetActive(true);
		collisionModel->LocalBounds() = *(impl->collider);
		collisionModel->SetOrigin(orthoOrigin);
		
		// FIXME: use an eTransform and offset instead of directly linking visuals and colliders
		renderImage->RenderBlock() += (eVec3)collisionModel->LocalBounds()[0];	// FREEHILL 3d topological sort
	}
// FREEHILL END AABB (eBounds) collisionModel import test (2/2)

	// visual alignment with isometric grid
	const float isoCellWidthAdjustment = (float)game.GetMap().TileMap().IsometricCellWidth() * 0.5f;
	const float isoCellHeightAdjustment = (float)game.GetMap().TileMap().IsometricCellHeight();
	eVec2 renderImageOrigin = orthoOrigin;
	eMath::CartesianToIsometric(renderImageOrigin.x, renderImageOrigin.y);
	renderImageOrigin += eVec2(-isoCellWidthAdjustment, isoCellHeightAdjustment - (float)renderImage->GetImageFrame()->h);
	renderImage->SetOrigin(renderImageOrigin);
}