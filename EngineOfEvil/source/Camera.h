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
#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "Bounds.h"
#include "RenderTarget.h"

//***********************************************
//				eCamera 
// Mobile 2D Axis-Aligned Orthographic box 
// that draws eRenderImageIsometric objects to either
// of two rendering targets: a main and a debug.
// The debug target draws on top of the main target.
// DEBUG: Use eRenderer to draw on an eCamera
// and output to a window/rendering context,
// and to ensure eRenderTargets get cleared properly
//***********************************************
class eCamera : public eClass {
public:

	friend class eRenderer;							// for direct access to the cameraPool and cameraPoolInserts

public:


	void											Init(const eVec2 & size, const eVec2 & worldPosition, float zoomLevel = 1.0f, float panSpeed = defaultCamSpeed);
	void											Think();
	const eBounds &									AbsBounds() const;
	void											SetOrigin(const eVec2 & newOrigin);
	const eVec2 &									GetOrigin() const;
	float											GetZoom() const;
	void											ZoomIn();
	void											ZoomOut();
	void											ResetZoom();
	void											SetSize(const eVec2 & newSize);
	bool											Moved() const;
	eVec2											ScreenToWorldPosition(const eVec2 & screenPoint) const;
	eVec2											MouseWorldPosition() const;
	eRenderTarget * const							GetRenderTarget();
	eRenderTarget * const							GetDebugRenderTarget();

	virtual int										GetClassType() const override				{ return CLASS_CAMERA; }
	virtual bool									IsClassType(int classType) const override	{ 
														if(classType == CLASS_CAMERA) 
															return true; 
														return eClass::IsClassType(classType); 
													}
private:

	void											UpdateZoom();

public:

	// FIXME: load these from engine config file
	static constexpr const float					zoomSpeed		= 0.2f;
	static constexpr const float					maxZoom			= 2.0f;
	static constexpr const float					minZoom			= 0.4f;				
	static constexpr const float					defaultCamSpeed = 20.0f;

private:
	
	std::vector<eRenderImageIsometric *>			cameraPoolInserts;				// minimizes priority re-calculations of dynamic vs. static eGameObjects
	std::vector<eRenderImageIsometric *>			cameraPool;						// game-world that moves and scales with this camera's renderTargets
	eRenderTarget									renderTarget;					// move and scale with this->absBounds, with draw-order sorting based on eRenderImageIsometric::renderBlock		
	eRenderTarget									debugRenderTarget;				// move and scale with this->absBounds, but draw last, without draw-order sorting (same size and origin as renderTarget)
	eBounds											absBounds;						// access to renderTarget size and position within the main rendering context
	eVec2											defaultSize;					// allows zoom in/out with minimal precision-loss, and allows visible area resize w/o zoom
	float											panSpeed;
	bool											moved;
};	

#endif /* EVIL_CAMERA_H */

