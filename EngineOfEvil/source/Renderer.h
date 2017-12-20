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
#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "RenderImage.h"
#include "Camera.h"

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
// CameraPool draws to a scalable target that typically tracks with the camera
// OverlayPool draws so a non-scalable target that typically draws to screen coordinates
// DEBUG: RENDERTYPE_STATIC items must always be drawn last 
// otherwise they will be overwritten by the scalable target
// and its RENDERTYPE_DYANMIC items
//***************************************************
class eRenderer : public eClass {
public:
					
	bool								Init(const char * name = "Engine of Evil", int windowWidth = 1500, int windowHeight = 800);
	void								Shutdown() const;
	void								Show() const;
	SDL_Rect							ViewArea() const;

	SDL_Renderer * const				GetSDLRenderer() const;
	SDL_Window * const					GetWindow() const;

	void								PollEvents() const;
	bool								RegisterCamera(eCamera * newCamera);
	bool								UnregisterCamera(eCamera * camera);
	void								UnregisterAllCameras();
	int									NumRegisteredCameras() const;
	bool								AddToCameraRenderPool(eCamera * registeredCamera, eRenderImage * renderImage);
	bool								AddToOverlayRenderPool(eRenderImage * renderImage);
	void								Flush();

	void								DrawOutlineText(eRenderTarget * target, const char * text, eVec2 & point, const SDL_Color & color, bool constText);
	void								DrawImage(eRenderImage * renderImage) const;
	void								DrawLines(eRenderTarget * target, const SDL_Color & color, std::vector<eVec2> points);
	void								DrawIsometricPrism(eRenderTarget * target, const SDL_Color & color, const eBounds3D & rect);
	void								DrawIsometricRect(eRenderTarget * target, const SDL_Color & color, const eBounds & rect);
	void								DrawCartesianRect(eRenderTarget * target, const SDL_Color & color, const eBounds & rect, bool fill);

	eRenderTarget * const				GetDefaultOverlayTarget();
	eRenderTarget * const				GetDebugOverlayTarget();

	virtual int							GetClassType() const override				{ return CLASS_RENDERER; }
	virtual bool						IsClassType(int classType) const override	{ 
											if(classType == CLASS_RENDERER) 
												return true; 
											return eClass::IsClassType(classType); 
										}

	static void							TopologicalDrawDepthSort(const std::vector<eRenderImage *> & renderImagePool);

private:

	bool								CheckDrawnStatus(eRenderTarget * renderTarget, eRenderImage * renderImage) const; 
	void								SetRenderTarget(eRenderTarget *);
	void								FlushCameraPool(eCamera * registeredCamera);
	void								FlushOverlayPool();

	static void							VisitTopologicalNode(eRenderImage * renderImage);

private:

	static int							globalDrawDepth;								// for eRenderer::TopologicalDrawDepthSort

	std::vector<eRenderImage *>			overlayPool;									// images static to the screen regardless of camera position
	std::vector<eRenderImage *>			overlayPoolInserts;								// minimize priority re-calculations	
	std::vector<eCamera *>				registeredCameras;								// cameras to copy to the main rendering context during Flush

	SDL_Window *						window;
	SDL_Renderer *						internal_renderer;

	// swappable render targets
	// draw-order sorting based on eRenderImage::renderBlock worldPositions
	eRenderTarget						defaultOverlayTarget;							// DEBUG: default SDL rendertarget (nullptr) is the window texture
	eRenderTarget						debugOverlayTarget;								// draw straight on the window (no camera adjustment), last, without draw-order sorting
	eRenderTarget *						currentRenderTarget	= &defaultOverlayTarget;	// current target being drawn to
	
	TTF_Font *							font;											// FIXME: only one test font for this rendering context for now
};

#endif /* EVIL_RENDERER_H */
