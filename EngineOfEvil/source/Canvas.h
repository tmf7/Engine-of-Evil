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
#ifndef EVIL_CANVAS_H
#define EVIL_CANVAS_H

#include "GameObject.h"
#include "RenderTarget.h"

namespace evil {

class eCamera;
class eRenderImageBase;
class eRenderImageIsometric;

enum class CanvasType {
	SCREEN_SPACE_OVERLAY,
	CAMERA_SPACE_OVERLAY,
	WORLD_SPACE
};

//***********************************************
//				eCanvas 
// Mobile 2D texture for drawing eRenderImageBase objects.
// Registered to either the main rendering context as an overlay,
// a specific camera as an overlay, or given a eRenderImageIsometric
// that uses the eRenderTarget's texture as the eImage,
// and a renderblock the size of the texture (with minimal depth)
// at the current eGameObject::orthoOrigin converted to isometric worldspace
//***********************************************
class eCanvas : public eGameObject {

	ECLASS_DECLARATION(eCanvas)

public:

	void											Init(const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale = vec2_one, CanvasType type = CanvasType::SCREEN_SPACE_OVERLAY, eCamera * cameraToOverlay = nullptr);
	bool											AddToRenderPool(eRenderImageBase * renderImage);
	void											ClearRenderPools();
	void											Flush();

	virtual void									Think() override;

private:
	
	// DEBUG: both of these reference members have lifetimes dictated by their eGameObject owner (ie: *this)
	// DEBUG: this aggregation prevents the compiler from generating an assignment operator
	// TODO: if needed, write the assignment operator to perform new GetComponent<> on the new object's reference members
	eRenderTarget &									renderTarget;
	eRenderImageIsometric &							renderImage;

	std::vector<eRenderImageBase *>					dynamicPool;					// dynamic eGameObjects to draw, minimizes priority re-calculations of dynamic vs. static eGameObjects
	std::vector<eRenderImageBase *>					staticPool;						// static eGameObjects that scale with this eCanvas
	eCamera *										targetCamera	= nullptr;		// eCamera to resize with and overlay (to which *this is registered)
	CanvasType										canvasType		= CanvasType::SCREEN_SPACE_OVERLAY;
};	

}      /* evil */
#endif /* EVIL_CAMERA_H */
