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

#include "Canvas.h"
#include "RenderImageIsometric.h"

namespace evil {

//***********************************************
//				eCamera 
// Mobile 2D Axis-Aligned Orthographic box 
// that draws eRenderImageIsometric objects to
// its eRenderTarget texture during eRenderer::Flush
// with draw-order sorting based on eRenderImageIsometric::renderBlock worldPositions.
// Camera-Overlay type eCanvases also get registered and flushed on *this
// adds uniform scaling (zoom) functionality to eRenderTarget,
// and converts screen coordinates into isometric-world space
//***********************************************
class eCamera : public eGameObject {

	ECLASS_DECLARATION(eCamera)

public:

	void											Configure(const eVec2 & size, const eVec2 & worldPosition, float zoomLevel = 1.0f, float panSpeed = defaultCamSpeed);
	void											Think();
	void											ZoomIn();
	void											ZoomOut();
	void											SetZoom(float newZoomLevel);
	float											GetZoom() const;
	bool											Moved();
	eVec2											ScreenToWorldPosition(const eVec2 & screenPoint) const;
	eVec2											MouseWorldPosition() const;
	bool											AddToRenderPool(eRenderImageIsometric * renderImage);
	void											ClearRenderPools();

	bool											RegisterOverlayCanvas(eCanvas * newOverlay);
	bool											UnregisterOverlayCanvas(eCanvas * overlay);
	void											UnregisterAllOverlayCanvases();
	int												NumRegisteredOverlayCanvases() const;

	void											Flush();

public:

	// FIXME: load these from engine config file
	static constexpr const float					zoomSpeed		= 0.2f;
	static constexpr const float					maxZoom			= 2.0f;
	static constexpr const float					minZoom			= 0.4f;				
	static constexpr const float					defaultCamSpeed = 20.0f;

private:
	
	std::vector<eRenderImageIsometric *>			dynamicPool;				// dynamic eGameObjects to draw, minimizes priority re-calculations of dynamic vs. static eGameObjects
	std::vector<eRenderImageIsometric *>			staticPool;					// static eGameObjects that move and scale with this camera's renderTargets
	std::vector<eCanvas *>							registeredOverlays;			// eCanvases to draw after the staticPool and dynamicPool have flushed
	float											panSpeed;
};	

}      /* evil */
#endif /* EVIL_CAMERA_H */

