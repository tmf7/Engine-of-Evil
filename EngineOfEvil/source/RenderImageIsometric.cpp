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
#include "Map.h"
#include "RenderImageIsometric.h"

//*************
// eRenderImageIsometric::eRenderImageIsometric
// DEBUG: defaults initialSpriteFrame outside [0, image->NumSubFrames) to 0
//*************
eRenderImageIsometric::eRenderImageIsometric(eGameObject * owner, const std::shared_ptr<eImage> & initialImage, const eVec3 & renderBlockSize, int initialImageFrame, const eVec2 & offset, bool isPlayerSelectable)
	: eRenderImageBase(owner, initialImage, initialImageFrame, offset, isPlayerSelectable) {
	SetRenderBlockSize(renderBlockSize);
}

//*************
// eRenderImageIsometric::~eRenderImageIsometric
//*************
eRenderImageIsometric::~eRenderImageIsometric() {
	ClearAreas();
}

//************
// eRenderImageIsometric::UpdateRenderBlock
// snaps the renderblock minimum z value according to owner::worldlayer, and
// the x,y is positioned using either owner::collisionModel (if non-nullptr), or owner::orthoOrigin,
// to best prevent renderBlock overlap and minimize draw-order sorting anomolies, otherwise the  is used
//************
void eRenderImageIsometric::UpdateRenderBlock() {
	auto & renderBlockMins = renderBlock[0];

	// static eGameObjects snap z according to worldLayer
	// dynamic eGameObjects use the fluid zPosition
	if (owner->WorldLayerDelta()) {
		if (owner->IsStatic()) {
			const float newRBMinZ = (float)owner->GetMap()->TileMap().MinZPositionFromLayer(owner->GetWorldLayer());
			renderBlock += eVec3(0.0f, 0.0f, newRBMinZ - renderBlockMins.z);
		} else {
			renderBlock += eVec3(0.0f, 0.0f, owner->GetZPosition() - renderBlockMins.z);
		}
	}

	if (&owner->CollisionModel() != nullptr) {
		auto & collisionMins = owner->CollisionModel().AbsBounds()[0];
		renderBlock += eVec3(collisionMins.x - renderBlockMins.x, collisionMins.y - renderBlockMins.y , 0.0f);
	} else {
		auto & ownerOrigin = owner->GetOrigin();
		renderBlock += eVec3(ownerOrigin.x - renderBlockMins.x, ownerOrigin.y - renderBlockMins.y , 0.0f);
	}
}

//*************
// eRenderImageIsometric::SetRenderBlockSize
//*************
void eRenderImageIsometric::SetRenderBlockSize(const eVec3 & newSize) {
	auto & renderBlockMins = renderBlock[0];
	renderBlock = eBounds3D(renderBlockMins, renderBlockMins + newSize);
}

//************
// eRenderImageIsometric::Update
// ensures this->origin tracks along with owner::orthoOrigin
// ensures only the visuals are isometric, 
// while backend collision occurs on a 2D top-down grid
// DEBUG: if owner->IsStatic this only calls UpdateAreas during loadtime initialization, not each frame
//************
void eRenderImageIsometric::Update() {
	oldOrigin = origin;
	origin = owner->GetOrigin();	
	eMath::CartesianToIsometric(origin.x, origin.y);
	origin += ownerOriginOffset;

	UpdateWorldClip();
	if (origin != oldOrigin || (owner->IsStatic() && game->GetGameTime() < 5000)) {
		if (isSelectable)
			UpdateAreasWorldClipArea();
		else
			UpdateAreasWorldClipCorners();
	}

	UpdateRenderBlock();
}

//************
// eRenderImageIsometric::ClearAreas
// removes this from all eMap::tileMap gridcells with pointers to it
// and clear this->areas gridcell pointers
//************
void eRenderImageIsometric::ClearAreas() {
	for (auto && cell : areas) {
		auto & contents = cell->RenderContents();
		if (contents.empty())	// FIXME: necessary to prevent a shutdown crash using std::unordered_map::find (insead of std::find) because of index reference
			continue;

		auto & index = contents.find(this);
		if (index != contents.end())
			contents.erase(index);
	}
	areas.clear();
}

//***************
// eRenderImageIsometric::UpdateAreasWorldClipCorners
// adds this to the eMap::tileMap gricells that contain the four corners of this->worldClip
// and adds those same gridcell pointers to this->areas
// DEBUG(performance): ensures no renderImage suddenly dissappears when scrolling the camera
// for a single tileMap layer this results in each eGridCell::renderContents::size of:
// 4 : 6 : 8, for center : edge : corner (parts of the tileMap) on average
// more layers increases sizes (eg: 3 layers is about 4-6 : 11 : 20, depending on map design)
//***************
void eRenderImageIsometric::UpdateAreasWorldClipCorners() {
	ClearAreas();

	std::array<eVec2, 4> visualWorldPoints;
	worldClip.ToPoints(visualWorldPoints.data());

	// clip rectangle to orthographic world-space for proper grid alignment
	auto & tileMap = owner->GetMap()->TileMap();
	for (auto & point : visualWorldPoints) {
		eMath::IsometricToCartesian(point.x, point.y);
		auto & cell = tileMap.IndexValidated(point);
		auto & searchContents = cell.RenderContents();
		if (searchContents.find(this) == searchContents.end()) {	// don't add the same renderImage or cell twice
			searchContents[this] = this;
			areas.emplace_back(&cell);
		}
	}
}

//***************
// eRenderImageIsometric::UpdateAreasWorldClipArea
// adds *this to the eMap::tileMap gricells that this->worldClip overlaps
// and adds those same gridcell pointers to this->areas
// DEBUG(performance): this fn relies on the expensive eCollision::OBBOBBTest
// instead of essentially-no collision test (see: eRenderImageIsometric::UpdateAreasWorldClipCorners)
//***************
void eRenderImageIsometric::UpdateAreasWorldClipArea() {
	ClearAreas();

	eVec2 corner = worldClip[0];
	eVec2 xAxis(worldClip[1].x, worldClip[0].y);
	eVec2 yAxis(worldClip[0].x, worldClip[1].y);
	std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
	for (auto & point : obbPoints)
		eMath::IsometricToCartesian(point.x, point.y);

	const eBox worldClipArea(obbPoints.data());
	eCollision::GetAreaCells(owner->GetMap(), worldClipArea, areas);
	for (auto && cell : areas)
		cell->RenderContents()[this] = this;
}