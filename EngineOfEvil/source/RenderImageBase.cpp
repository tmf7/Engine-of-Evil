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
#include "RenderImageBase.h"

using namespace evil;

ECLASS_DEFINITION(eComponent, eRenderImageBase)
ECOMPONENT_DEFINITION(eRenderImageBase)

//*************
// eRenderImageBase::eRenderImageBase
// DEBUG: defaults initialSpriteFrame outside [0, image->NumSubFrames) to 0
//*************
eRenderImageBase::eRenderImageBase(eGameObject * owner, const std::shared_ptr<eImage> & initialImage, int initialImageFrame, const eVec2 & offset, bool isPlayerSelectable)
	: image(initialImage),
	  ownerOriginOffset(offset),
	  isSelectable(isPlayerSelectable) {

	this->owner = owner;
	SetImageFrame(initialImageFrame < 0 || initialImageFrame > image->NumSubframes() ? 0 
																					 : initialImageFrame);
}

//*************
// eRenderImageBase::SetImage
// DEBUG: no range checking for faster assignment
//*************
void eRenderImageBase::SetImage(int imageManagerIndex) {
	image = game->GetImageManager().GetByResourceID(imageManagerIndex);
}

//*************
// eRenderImageBase::SetImageFrame
// DEBUG: assumes image has been initialized
// DEBUG: no range checking for faster assignment
//*************
void eRenderImageBase::SetImageFrame(int subframeIndex) {
	srcRect = &image->GetSubframe(subframeIndex);
}

//*************
// eRenderImageBase::GetOverlapImageFrame
// intersects this->worldClip with otherWorldClip
// and returns the ImageFrame needed to draw a sub-section of the current ImageFrame
// DEBUG: at most a copy of the original ImageFrame would be returned
//*************
SDL_Rect eRenderImageBase::GetOverlapImageFrame(const eBounds & otherWorldClip) const {
	eBounds subRect = worldClip.Intersect(otherWorldClip);
	eVec2 srcRectOrigin((float)srcRect->x, (float)srcRect->y);
	subRect.TranslateSelf(srcRectOrigin - origin);					// worldClip[0]
	return SDL_Rect { eMath::NearestInt( subRect[0].x ) , 
					  eMath::NearestInt( subRect[0].y ) , 
					  eMath::NearestInt( subRect.Width() ) , 
					  eMath::NearestInt( subRect.Height() ) };
}

//*************
// eRenderImageBase::UpdateWorldClip
// DEBUG: only call this after ImageFrame has been assigned
//*************
void eRenderImageBase::UpdateWorldClip() {
	worldClip = eBounds(origin, origin + eVec2((float)srcRect->w * scale.x, (float)srcRect->h) * scale.y);
}

//************
// eRenderImageBase::Update
// ensures this->origin and worldClip tracks along with owner::orthoOrigin
//************
void eRenderImageBase::Update() {
	oldOrigin = origin;
	origin = owner->GetOrigin() + ownerOriginOffset;
	UpdateWorldClip();
}

//*************
// eRenderImageBase::SetOrigin
//*************
void eRenderImageBase::SetOrigin(const eVec2 & newOrigin) {
	owner->SetOrigin(newOrigin);
}


//***************
// eRenderImageBase::SetLastDrawnTime
// updates the lastDrawnTime and clears all
// drawnTo eCamera/eCanvas pointers if this is
// the first time *this is being drawn this frame
//***************
void eRenderImageBase::SetLastDrawnTime() {
	auto gameTime = game->GetGameTime();
	if (lastDrawnTime < gameTime) {
		lastDrawnTime = gameTime;
		drawnToCameras.clear();
		drawnToCanvases.clear();
	}
}

//***************
// eRenderImageBase::AddDrawnToItem
// updates the renderImage's lastDrawnTime according to eGame::gameTime if this is the first call this frame
// returns false if *this has already been assigned to param drawnToCamera, true otherwise
// DEBUG: it's quicker to do a linear search of the small drawnToCameras vector for an eCamera *,
// than it is to search an eCamera's larger renderPools for an eRenderImageBase *
//***************
bool eRenderImageBase::AddDrawnToItem(eCamera * drawnToCamera) {
	SetLastDrawnTime();

	bool notDrawnYet = std::find(	drawnToCameras.begin(), 
									drawnToCameras.end(), 
									drawnToCamera
								) == drawnToCameras.end();

	if (notDrawnYet)
		drawnToCameras.emplace_back(drawnToCamera);

	return notDrawnYet;
}

//***************
// eRenderImageBase::AddDrawnToItem
// updates the renderImage's lastDrawnTime according to eGame::gameTime if this is the first call this frame
// returns false if *this has already been assigned to param drawnToCanvas, true otherwise
// DEBUG: it's quicker to do a linear search of the small drawnToCanvases vector for an eCanvas *,
// than it is to search an eCanvas's larger renderPools for an eRenderImageBase *
//***************
bool eRenderImageBase::AddDrawnToItem(eCanvas * drawnToCanvas) {
	SetLastDrawnTime();

	bool notDrawnYet = std::find(	drawnToCanvases.begin(), 
									drawnToCanvases.end(), 
									drawnToCanvas
								) == drawnToCanvases.end();

	if (notDrawnYet)
		drawnToCanvases.emplace_back(drawnToCanvas);

	return notDrawnYet;
}