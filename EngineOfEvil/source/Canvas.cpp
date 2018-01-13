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
//***************
void eCanvas::Init(eMap * onMap, const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale, CanvasType type, eCamera * cameraToOverlay ) {
	auto & renderer = game->GetRenderer();
	const auto & context = renderer.GetSDLRenderer();
	canvasType = type;
	map = onMap;
	SDL_Point intSize = { eMath::NearestInt(size.x), eMath::NearestInt(size.y) };
	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);

	// FIXME(?): possibly only add this for worldspace eCanvas types,
	// and let overlays copy straight onto a modified scale mainRenderTarget
	// ...although, eCanvas would need to save the size and scale 
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
			auto & initialImage = std::make_unique<eImage>(renderTarget.GetTargetTexture(), "WorldSpaceCanvasInstance", INVALID_ID);
			initialImage->SetSubframes( std::vector<SDL_Rect> ( { SDL_Rect { 0, 0, intSize.x, intSize.y } } ) );
			AddComponent< eRenderImageIsometric >( this, initialImage, eVec3( size.x, 2.0f, size.y ) );
			renderImage = GetComponent< eRenderImageIsometric >();
			return;				
		}
	}
}

//***************
// eCanvas::Think
// FIXME: make sure this eCanvas calls eRenderImageIsometric::Update() whenever it moves or Resizes (so it gets added to the eMap appropriately)
// FIXME: should an eCanvas::Resize also resize the window or camera it overlays? (only canvas.renderTarget.scale scales items, not eCanvas::Resize)
// FIXME: if an eCamera with several registered eCanvas overlays Resizes, how should the eCanvases get Resized? During eCanvas::Think?
// SOLUITON(?): apply a non-destructive calculated OFFSET to renderPool item origins (eg: if the canvas has been Resized)...ANCHOR points (and default top-left image PIVOTs/origin)
//***************
void eCanvas::Think() {
	switch(canvasType) {
		case CanvasType::SCREEN_SPACE_OVERLAY: {
			// TODO: get the current window size and Resize (DON'T resize the renderTexture)
		}

		case CanvasType::CAMERA_SPACE_OVERLAY: { 
			// TODO: get the current camera size???? and Resize (DON'T resize the renderTexture)
		}

		case CanvasType::WORLD_SPACE: {
			return;							// TODO(?): is there something else the canvas needs to do in worldspace besides have its renderImage Updated
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
	if (!renderTarget.Validate())
		return;

	QuickSort(	staticPool.data(),
				staticPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

	auto & renderer = game->GetRenderer();
	renderer.SetRenderTarget(&renderTarget);

	const auto & canvasOffset = GetOrigin();
	for (auto && renderImage : staticPool)
		renderer.DrawImage(renderImage, canvasOffset);

	ClearRenderPools();

	renderer.SetRenderTarget(renderer.GetMainRenderTarget());
	SDL_RenderCopy(renderer.GetSDLRenderer(), renderTarget.GetTargetTexture(), NULL, NULL);
}