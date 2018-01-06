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
#include "Canvas.h"

//***************
// eCanvas::Configure
// DEBUG: use this fn instead of eRenderTarget::Init to initialize *this
//***************
void eCanvas::Configure(const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale, CanvasType type, eCamera * cameraToOverlay) {
	auto & renderer = game->GetRenderer();
	const auto & context = renderer.GetSDLRenderer();
	SDL_Point intSize = { eMath::NearestInt(size.x), eMath::NearestInt(size.y) };

//	eRenderTarget::Init( context, intSize.x, intSize.y, worldPosition, scale);
//	AddComponent<eRenderTarget>(context, intSize.x, intSize.y, worldPosition, scale);		// FIXME/TODO: ctor calls Init? either way AddComponent needs to verify the add was successful (bool or nullptr)


	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);

	switch(type) {
		case CanvasType::SCREEN_SPACE_OVERLAY: {
//			renderer.RegisterRenderTarget(GetComponent<eRenderTarget>());					// FIXME/TODO: implement eGameObject::GetComponent
			return;
		}

		case CanvasType::CAMERA_SPACE_OVERLAY: { 
			targetCamera = cameraToOverlay;
			targetCamera->RegisterOverlayCanvas(this);
			return; 
		}

		case CanvasType::WORLD_SPACE: {

//			AddComponent<eRenderImageIsometric>( this, renderImage->target, eVec3( size.x, 2.0f, size.y ) );		// FIXME: do this (w/target made into an unregistered eImage) instead of the worldSpaceRI hack

			// FIXME: should this get registered to the eImageManager?
			// SOLUTION: no.
			worldSpaceImage = std::make_unique<eImage>(target, "WorldSpaceCanvasInstance", INVALID_ID);
			worldSpaceImage->SetSubframes( std::vector<SDL_Rect> ( 
																	{ SDL_Rect { 0, 0, intSize.x, intSize.y } } 
																 ) 
										 );

			// FIXME: eRenderImageIsometric needs to have an eGameObject owner pointer, and to be Updated
			// SOLUTIN: ??? a dummy object? (still needs an eMap pointer though)
			// make a distinciton between world-space/gameworld and a map (ie: a map fills the gameworld)
			worldSpaceRenderImage = std::make_unique<eRenderImageIsometric> ( nullptr, 
																			  worldSpaceImage, 
																			  eVec3( size.x, 2.0f, size.y ) );

			// FIXME: do this whenever *this moves or resizes (so it gets added to the eMap appropriately)
			worldSpaceRenderImage->Update();
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
	if (renderImage->UpdateDrawnStatus(this))
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
	if (!visible)
		return;

	QuickSort(	staticPool.data(),
				staticPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

	auto & renderer = game->GetRenderer();
	renderer.SetRenderTarget(this);

	for (auto && renderImage : staticPool)
		renderer.DrawImage(renderImage);

	ClearRenderPools();

	renderer.SetRenderTarget(renderer.GetMainRenderTarget());
	SDL_RenderCopy(renderer.GetSDLRenderer(), target, NULL, NULL);
}