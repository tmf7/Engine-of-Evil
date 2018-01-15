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
#include "Camera.h"
#include "RenderImageIsometric.h"

using namespace evil;

ECLASS_DEFINITION(eGameObject, eCanvas)

//***************
// eCanvas::Init
// FIXME: what about zPosition/worldLayer, especially given that camera's and canvases need to be sorted to begin with
// FIXME: size, and x/y of worldPosition are meaningless to overlays as they are driven by their owners' sizes
// SOLUTION: create InitAsWorldSpace, InitAsScreenOverlay, InitAsCameraOverlay that can each call a private Init for common functionality
//***************
void eCanvas::Init(eMap * onMap, const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale, CanvasType type, eCamera * cameraToOverlay ) {
	auto & renderer = game->GetRenderer();
	const auto & context = renderer.GetSDLRenderer();
	canvasType = type;
	map = onMap;
	SDL_Point intSize = { eMath::NearestInt(size.x), eMath::NearestInt(size.y) };
	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);

	AddComponent<eRenderTarget>(this, context, intSize.x, intSize.y, scale);
	renderTarget = GetComponent<eRenderTarget>();
	
	switch(type) {
		case CanvasType::SCREEN_SPACE_OVERLAY: {
			renderer.RegisterOverlayCanvas(this);
			return;
		}

		case CanvasType::CAMERA_SPACE_OVERLAY: { 
			targetCamera = cameraToOverlay;						// FIXME/BUG: user may neglect to set cameraToOverlay
			targetCamera->RegisterOverlayCanvas(this);			// SOLUTION: make Init a bool return value, log an error, set bool intialized like eCamera
			return; 
		}

		case CanvasType::WORLD_SPACE: {
			// DEBUG: eRenderImageIsometric maintains the lifetime of eRenderTarget::target via its std::shared_ptr<eImage>,
			// meaning eImageManager doesn't need to manage this image because it wil only ever be used by *this, and no other eGameObject
			auto & initialImage = std::make_unique<eImage>(renderTarget->GetTargetTexture(), "WorldSpaceCanvasInstance", INVALID_ID);
			initialImage->SetSubframes( std::vector<SDL_Rect> ( { SDL_Rect { 0, 0, intSize.x, intSize.y } } ) );
			AddComponent< eRenderImageIsometric >( this, initialImage, eVec3( size.x, 2.0f, size.y ) );
			renderImage = GetComponent< eRenderImageIsometric >();
			renderImage->SetOrigin(worldPosition);
			return;				
		}
	}
}

//***************
// eCanvas::Resize
//***************
bool eCanvas::Resize(int newWidth, int newHeight) {
	if (canvasType != CanvasType::WORLD_SPACE)				// TODO: log an error "Trying to resize an eCamera|Screen overlay eCanvas"
		return false;

	// TODO: eCanvas::Resize should only be allowed for worldspace canvases, while overlay sizes a driven by their owners 
	// FIXME: resizing the renderTarget should also resize the renderImageIsometric's renderblock
	// FIXME: resizing the worldspace eCanvas in Unity doesn't scale what's on it, only setting the child size|scale values (or parent scale value) does
	// ... meaning the renderTarget DOES get resized, and the drawing scale of the canvas is additionally affected by the camSize (rel. to the window) as usual
	// ... however ... if drawing new items on the resized (not scaled) canvas ... then the coordinate range would change... hence the use for anchor/pivot/offsets
}

//***************
// eCanvas::Think

// SOLUTION: Resizing an eCamera with several registered eCanvas overlays does nothing to the canvases at that point, but during eCanvas::Flush the canvas' renderTarget scale is adjusted.
// IE: camScale = windowSize / camSize; and camOverlayScale = camSize / canvasSize; during eCamera::Flush and eCanvas::Flush, respectively.
// SIMILARLY: windowOverlayScale = windowSize / canvasSize; during eCanvas::Flush

// TODO(~): resize the renderTarget to match the window size so nothing gets clipped, which requires calculating new coordinates for renderPool items. 
// TODO(~): bool scaleWithWindow = true; (ie const renderTexture size just scaled), and = false (ie variable renderTexture size and const scale, w/anchor point offsets for renderPool items)
// SOLUITON(~): apply a non-destructive calculated OFFSET to renderPool item origins (eg: if the canvas has been Resized)
// IE: ANCHOR and PIVOT points are NORMALIZED positions within the parent and child (so 0.5f, 0.5f is the center and 1,1 is th bottom-right)
// THEN x/y positions of elements' top-left corner == (ANCHOR * parent dims) + (PIVOT * child dims) + offset
// HOWEVER: generally, the offset must always be within the bounds of the parent eCanvas size for the child to be visible
// SOLUTION(~): RectTransform eComponent that contains all these anchor/pivot/size values (instead of an eTransform, or eCollisionModel, or eBounds)

//***************
void eCanvas::Think() {
	switch(canvasType) {
		case CanvasType::SCREEN_SPACE_OVERLAY: {
			// TODO: get the current window size and re-scale the renderTarget

		}

		case CanvasType::CAMERA_SPACE_OVERLAY: { 
			// TODO: get the current camera size???? and Resize (should the renderTexture also resize to match the main renderTexure so nothing gets clipped?)
		}

		case CanvasType::WORLD_SPACE: {
			renderImage->Update();				// TODO(?): is there something else the canvas needs to do? eCamera already scales its observations of this up if the window stretches
			return;							
		}
	}
}

//***************
// eCanvas::AddToRenderPool
// adds param renderImage to either staticPool or dynamicPool,
// which later renders during eRenderer::Flush (if the camera is registered to the eRenderer)
// returns true if param renderImage hasn't already been added to this eCamera's renderPool
// returns false otherwise
// DEBUG: doesn't check if the eCamera is genuinely registered with the eRenderer, 
// if not, then param renderImage won't be drawn to the rendering context
// and this eCamera's pools won't be cleared (as happens during Flush)
//***************
bool eCanvas::AddToRenderPool(eRenderImageBase* renderImage) {
	if (!renderImage->AddDrawnToItem(this))
		return false;

	const auto & renderPool = (renderImage->Owner()->IsStatic() ? &staticPool : &dynamicPool);
	renderPool->emplace_back(renderImage);
	return true;
}

//**************
// eCanvas::ClearRenderPools
//**************
void eCanvas::ClearRenderPools() {
	staticPool.clear();
	dynamicPool.clear();
}

//**************
// eCanvas::Flush
// draws all the currently queued eRenderImageBase objects to
// this eCanvas's eRenderTarget texture, if any
//**************
void eCanvas::Flush() {
	if (!renderTarget->Validate())
		return;

	QuickSort(	staticPool.data(),
				staticPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

	auto & renderer = game->GetRenderer();
	renderer.SetRenderTarget(renderTarget);

	const auto & canvasOffset = GetOrigin();
	for (auto && renderImage : staticPool)
		renderer.DrawImage(renderImage, canvasOffset);

	ClearRenderPools();

	renderer.SetRenderTarget(renderer.GetMainRenderTarget());
	SDL_RenderCopy(renderer.GetSDLRenderer(), renderTarget->GetTargetTexture(), NULL, NULL);
}