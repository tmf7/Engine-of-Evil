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
#ifndef EVIL_RENDERIMAGE_H
#define EVIL_RENDERIMAGE_H

#include "Definitions.h"
#include "Bounds.h"
#include "Bounds3D.h"
#include "Sort.h"
#include "Image.h"
#include "Component.h"

class eRenderTarget;
class eGridCell;

//**************************************************
//				eRenderImage
// data used by eRenderer for draw-order sorting
// and drawing to the render target
//**************************************************
class eRenderImage : public eComponent {
private:

	friend class eRenderer;				// directly sets dstRect, priority, lastDrawnTime, allBehind, visited (no other accessors outside *this)

public:

												eRenderImage(eGameObject * owner);
	virtual									   ~eRenderImage() override;
	
	std::shared_ptr<eImage> &					Image();
	const std::shared_ptr<eImage> &				GetImage() const;
	void										SetImage(int imageManagerIndex);
	void										SetImageFrame(int subframeIndex);
	const SDL_Rect *							GetImageFrame() const;
	SDL_Rect									GetOverlapImageFrame(const eBounds & otherWorldClip) const;
	void										SetOrigin(const eVec2 & newOrigin);
	const eVec2 &								Origin() const;
	const eVec2 &								Offset() const;
	void										SetOffset(const eVec2 & newOffset);
	void										SetRenderBlockSize(const eVec3 & newSize);
	const eBounds3D &							GetRenderBlock() const;
	const eBounds &								GetWorldClip() const;
	const std::vector<eGridCell *> &			Areas() const;
	void										SetIsSelectable(bool isSelectable);
	bool										IsSelectable() const;

	virtual void								Update() override;
	virtual std::unique_ptr<eComponent>			GetCopy() const	override					{ return std::make_unique<eRenderImage>(*this); }
	virtual int									GetClassType() const override				{ return CLASS_RENDERIMAGE; }
	virtual bool								IsClassType(int classType) const override	{ 
													if(classType == CLASS_RENDERIMAGE) 
														return true; 
													return eComponent::IsClassType(classType); 
												}

private:

	void										UpdateWorldClip();
	void										UpdateRenderBlock();
	void										ClearAreas();
	void										UpdateAreasWorldClipCorners();
	void										UpdateAreasWorldClipArea();

private:

	std::vector<eRenderTarget *>				drawnTo;						// prevent attempts to draw this more than once per renderTarget per frame
	std::vector<eRenderImage *>					allBehind;						// topological sort
	std::vector<eGridCell *>					areas;							// the gridcells responsible for drawing *this
	std::shared_ptr<eImage>						image			= nullptr;		// source image (ie texture wrapper)
	eBounds3D									renderBlock;					// determines draw order of visible images
	eBounds										worldClip;						// dstRect in world space (ie: not adjusted with camera position yet) used for occlusion tests
	const SDL_Rect *							srcRect			= nullptr;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect									dstRect;						// SDL consumable cliprect, where on the screen (adjusted with camera position)
	eVec2										origin;							// top-left corner of image using world coordinates (not adjusted with camera position)
	eVec2										oldOrigin;						// minimizes number of UpdateAreas calls for non-static eGameObjects that aren't moving
	eVec2										orthoOriginOffset;				// offset from (eGameObject)owner::orthoOrigin (default: (0,0))
	float										priority;						// determined during topological sort, lower priority draws first
	Uint32										lastDrawnTime	= 0;			// allows the drawnTo vector to be cleared before *this is drawn the first time during a frame
	bool										isSelectable	= false;		// if this should added to all eGridCells its worldClip overlaps, or just its corners
	bool										visited			= false;		// topological sort
};

//*************
// eRenderImage::eRenderImage
//*************
inline eRenderImage::eRenderImage(eGameObject * owner) {
	this->owner = owner;
}

//*************
// eRenderImage::Image
//*************
inline std::shared_ptr<eImage> & eRenderImage::Image() {
	return image;
}

//*************
// eRenderImage::GetImage
//*************
inline const std::shared_ptr<eImage> & eRenderImage::GetImage() const {
	return image;
}

//*************
// eRenderImage::ImageFrame
// DEBUG: assumes image has been initialized
// DEBUG: no range checking for faster assignment
//*************
inline void eRenderImage::SetImageFrame(int subframeIndex) {
	srcRect = &image->GetSubframe(subframeIndex);
}

//*************
// eRenderImage::ImageFrame
//*************
inline const SDL_Rect * eRenderImage::GetImageFrame() const {
	return srcRect;
}

//*************
// eRenderImage::Origin
//*************
inline const eVec2 & eRenderImage::Origin() const {
	return origin;
}

//*************
// eRenderImage::Offset
// x and y distance from owner::orthoOrigin
//*************
inline const eVec2 & eRenderImage::Offset() const {
	return orthoOriginOffset;
}

//*************
// eRenderImage::SetOffset
// sets the x and y distance from owner::orthoOrigin
//*************
inline void eRenderImage::SetOffset(const eVec2 & newOffset) {
	orthoOriginOffset = newOffset;
}

//*************
// eRenderImage::GetRenderBlock
//*************
inline const eBounds3D & eRenderImage::GetRenderBlock() const {
	return renderBlock;
}

//*************
// eRenderImage::UpdateWorldClip
// DEBUG: only call this after ImageFrame has been assigned
//*************
inline void eRenderImage::UpdateWorldClip() {
	worldClip = eBounds(origin, origin + eVec2((float)srcRect->w, (float)srcRect->h));
}

//*************
// eRenderImage::GetWorldClip
//*************
inline const eBounds & eRenderImage::GetWorldClip() const {
	return worldClip;
}

//*************
// eRenderImage::Areas
//*************
inline const std::vector<eGridCell *> & eRenderImage::Areas() const {
	return areas;
}

//*************
// eRenderImage::SetIsSelectable
//*************
inline void eRenderImage::SetIsSelectable(bool isSelectable) {
	this->isSelectable = isSelectable;
}

//*************
// eRenderImage::IsSelectable
//*************
inline bool eRenderImage::IsSelectable() const {
	return isSelectable;
}

#endif /* EVIL_RENDERIMAGE_H */

