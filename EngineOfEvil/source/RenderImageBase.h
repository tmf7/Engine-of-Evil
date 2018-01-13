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
#ifndef EVIL_RENDERIMAGE_BASE_H
#define EVIL_RENDERIMAGE_BASE_H

#include "Definitions.h"
#include "Bounds.h"
#include "Image.h"
#include "Component.h"

// SOLUTION: eButton is an eGameObject, which will have an eRenderImageBase component,
// then eButton will have an eCanvas * drawTo pointer that will affect its orthoOrigin calculation (ie: a fixed position on the eCanvas)
// which in-turn affects the eRenderImageBase component position with respect to its eButton owner
// which means eRenderImageBase can have an orthoOriginOffset field...which should be ownerOriginOffset instead (for all)
// (otherwise eRenderImageBase::origin == owner->orthoOrigin for an eButton, which isn't too flexible)

namespace evil {

class eCamera;
class eCanvas;

//**************************************************
//				eRenderImageBase
// data used by eRenderer to draw textures to eRenderTargets
//**************************************************
class eRenderImageBase : public eComponent {

	ECLASS_DECLARATION(eRenderImageBase)
	ECOMPONENT_DECLARATION(eRenderImageBase)

public:

	friend class eRenderer;						// directly sets priority

public:

												eRenderImageBase(eGameObject * owner, const std::shared_ptr<eImage> & initialImage, int initialImageFrame = 0, const eVec2 & offset = vec2_zero, bool isPlayerSelectable = false);

	std::shared_ptr<eImage> &					Image();
	const std::shared_ptr<eImage> &				GetImage() const;
	void										SetImage(int imageManagerIndex);
	void										SetImageFrame(int subframeIndex);
	const SDL_Rect *							GetImageFrame() const;
	SDL_Rect									GetOverlapImageFrame(const eBounds & otherWorldClip) const;
	const eVec2 &								Offset() const;
	void										SetOffset(const eVec2 & newOffset);
	void										SetOrigin(const eVec2 & newOrigin);
	const eVec2 &								Origin() const;
	const eBounds &								GetWorldClip() const;
	void										SetIsSelectable(bool isSelectable);
	bool										IsSelectable() const;
	bool										AddDrawnToItem(eCamera * drawnToCamera);
	bool										AddDrawnToItem(eCanvas * drawnToCanvas);

	virtual void								Update() override;

protected:

	void										UpdateWorldClip();

private:

	void										SetLastDrawnTime();

protected:

	// only draw *this once per eCamera|eCanvas per frame
	std::vector<eCamera *>						drawnToCameras;
	std::vector<eCanvas *>						drawnToCanvases;

	std::shared_ptr<eImage>						image			= nullptr;		// source image (ie texture wrapper)
	eBounds										worldClip;						// scaled srcRect positioned with respect to origin, primarily used for occlusion tests
	eVec2										scale			= vec2_one;		// how much to up/down-scale the srcRect in x and y independently
	eVec2										origin;							// top-left corner of image using world coordinates (not adjusted with any eRenderTarget position)
	eVec2										oldOrigin;						// origin prior to most recent movement, if any
	eVec2										ownerOriginOffset;				// offset from (eGameObject)owner::orthoOrigin [default == (0,0)]
	const SDL_Rect *							srcRect			= nullptr;		// what part of the source image to draw (nullptr for all of it)
	float										priority;						// lower priority draws first to an eRenderTarget
	float										lastDrawnTime	= 0.0f;			// allows the drawnTo vector to be cleared before *this is drawn the first time during a frame
	bool										isSelectable	= false;		// TODO: use this to add to the grid...AND to "raytrace" against a UI element (ie don't for a disabled button...or just a plain ui image)
};

//*************
// eRenderImageBase::Image
//*************
inline std::shared_ptr<eImage> & eRenderImageBase::Image() {
	return image;
}

//*************
// eRenderImageBase::GetImage
//*************
inline const std::shared_ptr<eImage> & eRenderImageBase::GetImage() const {
	return image;
}

//*************
// eRenderImageBase::GetImageFrame
//*************
inline const SDL_Rect * eRenderImageBase::GetImageFrame() const {
	return srcRect;
}

//*************
// eRenderImageBase::Origin
//*************
inline const eVec2 & eRenderImageBase::Origin() const {
	return origin;
}

//*************
// eRenderImageBase::Offset
// x and y distance from owner::orthoOrigin
//*************
inline const eVec2 & eRenderImageBase::Offset() const {
	return ownerOriginOffset;
}

//*************
// eRenderImageBase::SetOffset
// sets the x and y distance from owner::orthoOrigin
//*************
inline void eRenderImageBase::SetOffset(const eVec2 & newOffset) {
	ownerOriginOffset = newOffset;
}

//*************
// eRenderImageBase::GetWorldClip
//*************
inline const eBounds & eRenderImageBase::GetWorldClip() const {
	return worldClip;
}

//*************
// eRenderImageBase::SetIsSelectable
//*************
inline void eRenderImageBase::SetIsSelectable(bool isSelectable) {
	this->isSelectable = isSelectable;
}

//*************
// eRenderImageBase::IsSelectable
//*************
inline bool eRenderImageBase::IsSelectable() const {
	return isSelectable;
}

}      /* evil */
#endif /* EVIL_RENDERIMAGE_BASE_H */