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
void eCanvas::Configure(const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale) {
	const auto & context = game->GetRenderer().GetSDLRenderer();
	SDL_Point intSize = { eMath::NearestInt(size.x), eMath::NearestInt(size.y) };
	eRenderTarget::Init( context, intSize.x, intSize.y, worldPosition, scale);
	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);
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