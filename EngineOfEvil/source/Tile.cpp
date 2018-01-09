/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#include "Game.h"
#include "Tile.h"
#include "Map.h"

using namespace evil;

ECLASS_DEFINITION(eClass, eTileImpl)
ECLASS_DEFINITION(eGameObject, eTile)

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
// rbSizeName: width height depth	# 0 index comment\n
// rbSizeName: width height depth	# 1\n
// (repeat for all default renderBlock sizes)
// # begin tile type definitions comment\n
// num_tiles: number of tiles\n
// imageFilename.eimg\n
// subframeIndex colliderType renderBlockType # master tile index comment\n	(0: no renderBlock/collider; non-0: value-1 for listed renderBlock/collider)
// (repeat tile property definitions for this file)
// # (end of tileset definition comment)\n
// imageFilename.eimg\n
// subframeIndex colliderType renderBlockType# master tile index comment\n
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
	if (!VerifyRead(read) || !game->GetImageManager().BatchLoad(buffer))
		return false;

	enum {
		LOADING_DEFAULT_COLLISION,
		LOADING_DEFAULT_RENDERBLOCKS,
		LOADING_TILESET
	};
	int readState = LOADING_DEFAULT_COLLISION;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// skip the third line comment
	std::vector<std::shared_ptr<eBounds>> defaultAABBList;

	while (readState == LOADING_DEFAULT_COLLISION) {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ':');							// collision shape text
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

	std::vector<eVec3> defaultRenderBlockSizes;
	while (readState == LOADING_DEFAULT_RENDERBLOCKS) {
		SkipFileKey(read);													// rbSize text
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
			SkipFileKey(read);												// num_tiles text
			readState = LOADING_TILESET;
		}
	} 

	// read how many tiles are about to be loaded
	// to minimize dynamic allocations
	std::size_t numTiles = 0;
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
		auto & sourceImage = game->GetImageManager().GetByFilename(buffer);
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

			tileSet.emplace_back(std::pair<int, int> { imageID, subframeIndex });				// FIXME: verify the subframe exists, 
																								// otherwise push an error image handle into this tileSet index
			int type = tileSet.size() - 1;
			tileTypes[type].tileType = type;
			if (colliderType > 0)
				tileTypes[type].collider = defaultAABBList[colliderType - 1];					// DEBUG: see rules for colliderType numbering in function header

			if (renderBlockType > 0)
				tileTypes[type].renderBlockSize = defaultRenderBlockSizes[renderBlockType - 1];	// DEBUG: see rules for renderBlockType numbering in function header

			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	read.close();
	return !tileSet.empty();
}

//************
// eTile::eTile
// owner is the originating eGridCell responsible for this eTile's lifetime
// origin is the top-left corner of the image position in world-coordinates (on an orthographic 2D grid)
// type is the index within the master tileSet vector
// layer sets this->renderImage.renderBlock's base z-position (for draw-order sorting)
// DEBUG: every tile is aligned with at least one cell no matter its size (ie: no freely-aligned tile-image origins)
//************
eTile::eTile(eGridCell * cellOwner, const eVec2 & origin, const int type, const Uint32 layer) 
	: cellOwner(cellOwner) {
	map = cellOwner->GetMap();
	SetWorldLayer(layer);

	// currently all tiles have an eRenderImageIsometric by default
	// DEBUG: the call to SetType duplicates some of the eRenderImageIsometric component initialization here, 
	// so the minimum complex arguments are used in this call
	AddComponent<eRenderImageIsometric>( this, 
										 game->GetImageManager().GetByResourceID(eTileImpl::tileSet.at(type).first),
										 eTileImpl::tileTypes[type].renderBlockSize
									   );
	SetType(type);
	SetOrigin(origin);
	UpdateComponents();
}

//************
// eTile::SetType
//************
void eTile::SetType(int newType) {
	// changing to a different tile type that may not have a collider
	RemoveComponent<eCollisionModel>();
	auto isoRenderImage = &GetComponent<eRenderImageIsometric>();

	impl = &eTileImpl::tileTypes[newType];																		// DEBUG: assumes newType is defined
	isoRenderImage->Image() = game->GetImageManager().GetByResourceID(eTileImpl::tileSet.at(newType).first);	// which image (tile atlas)
	isoRenderImage->SetImageFrame(eTileImpl::tileSet.at(newType).second);										// which part of that image

	// visual alignment with isometric grid
	const float isoCellWidthAdjustment = (float)map->TileMap().IsometricCellWidth() * 0.5f;
	const float isoCellHeightAdjustment = (float)map->TileMap().IsometricCellHeight();
	isoRenderImage->SetOffset(eVec2(-isoCellWidthAdjustment, isoCellHeightAdjustment - (float)isoRenderImage->GetImageFrame()->h));
	isoRenderImage->SetRenderBlockSize(impl->renderBlockSize);

	if (impl->collider != nullptr) 
		AddComponent<eCollisionModel>(this, *(impl->collider), vec2_zero, true);		// currently no collider offset and all tile colliders are set active
}