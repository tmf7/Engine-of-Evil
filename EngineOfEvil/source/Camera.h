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

namespace evil {

//***********************************************
//				eCamera 
// Mobile/Resizable/Zoomable 2D Axis-Aligned Orthographic box 
// that observes the game world and draws both
// its registered overlay eCanvases and what's within
// its bounds in the game world to the main rendering texture during eRenderer::Flush. 
// Performs draw-order sorting based on eRenderImageIsometric::renderBlock worldPositions.
// Performs conversions from mouse coordinates into isometric-world space coordinates.
//***********************************************
class eCamera : public eGameObject {

	ECLASS_DECLARATION(eCamera)

public:

	void											Init(eMap * onMap, const eVec2 & size, const eVec2 & worldPosition, float zoomLevel = 1.0f, float panSpeed = defaultCamSpeed);
	const eBounds &									AbsBounds() const;
	void											Resize(const eVec2 & newSize);
	void											ZoomIn();
	void											ZoomOut();
	void											SetZoom(float newZoomLevel);
	float											GetZoom() const;
	void											ResetZoom();
	bool											Moved() const;
	eVec2											ScreenToWorldPosition(const eVec2 & screenPoint) const;
	eVec2											MouseWorldPosition() const;
	bool											AddToRenderPool(eRenderImageIsometric * renderImage);
	void											ClearRenderPools();
	void											Flush();

	bool											RegisterOverlayCanvas(eCanvas * newOverlay);
	bool											UnregisterOverlayCanvas(eCanvas * overlay);
	void											UnregisterAllOverlayCanvases();
	int												NumRegisteredOverlayCanvases() const;

	virtual void									Think() override;

public:

	// FIXME: load these from engine config file
	static constexpr const float					zoomSpeed		= 0.2f;
	static constexpr const float					maxZoom			= 2.0f;
	static constexpr const float					minZoom			= 0.4f;				
	static constexpr const float					defaultCamSpeed = 20.0f;

private:

	// DEBUG: this reference member has a lifetime dictated by its eGameObject owner (ie: *this)
	// DEBUG: this aggregation prevents the compiler from generating an assignment operator
	// TODO: if needed, write the assignment operator to perform new GetComponent<> on the new object's reference members
	eCollisionModel &								collisionModel;

	std::vector<eRenderImageIsometric *>			dynamicPool;				// dynamic eGameObjects to draw, minimizes priority re-calculations of dynamic vs. static eGameObjects
	std::vector<eRenderImageIsometric *>			staticPool;					// static eGameObjects that move and scale with this camera's renderTargets
	std::vector<eCanvas *>							registeredOverlays;			// eCanvases to draw after the staticPool and dynamicPool have flushe
	eVec2											defaultSize;				// allows for uniform zoom in|out and zoom reset
	float											zoom;						// defaultSize multiplier
	float											panSpeed;
	bool											moved;						// if position or zoom has changed since the last frame
};	

}      /* evil */
#endif /* EVIL_CAMERA_H */

