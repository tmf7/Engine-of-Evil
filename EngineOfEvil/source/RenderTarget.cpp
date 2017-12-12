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
#include "RenderTarget.h"

const SDL_Color greyColor_trans		= { 128, 128, 128, SDL_ALPHA_TRANSPARENT };
const SDL_Color greyColor_opaque	= { 128, 128, 128, SDL_ALPHA_OPAQUE};
const SDL_Color blackColor			= { 0, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color greenColor			= { 0, 255, 0, SDL_ALPHA_OPAQUE };
const SDL_Color redColor			= { 255, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color blueColor			= { 0, 0, 255, SDL_ALPHA_OPAQUE };
const SDL_Color pinkColor			= { 255, 0, 255, SDL_ALPHA_OPAQUE };
const SDL_Color lightBlueColor		= { 0, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color yellowColor			= { 255, 255, 0, SDL_ALPHA_OPAQUE };

//***************************
// eRenderTarget::InitDefault
// initializes the default render target for a context
// DEBUG: only do this once per rendering context
//***************************
void eRenderTarget::InitDefault(SDL_Renderer * context, float zoomLevel) {
	this->context = context;
	this->zoomLevel = zoomLevel;
}

//***************************
// eRenderTarget::Init
// creates a new SDL_Texture within the given param context
// and sets its rendering position within the context to param contextPosition
// returns true on success, false on failure
//***************************
bool eRenderTarget::Init(SDL_Renderer * context, int width, int height, const eVec2 & contextPosition, float zoomLevel) {
	this->context = context;
	this->zoomLevel = zoomLevel;
	origin = contextPosition;
	target = SDL_CreateTexture( context,
								SDL_PIXELFORMAT_ARGB8888,						// DEBUG: this format may not work for all images
								SDL_TEXTUREACCESS_TARGET,
								width,
								height );

	if (!target)
		return false;

	// ensure the target can alpha-blend
	SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
	return true;
}

//***************************
// eRenderTarget::Resize
// creates a new SDL_Texture, 
// copies the current target contents
// onto it, then destroys the current target
// returns true on success,
// returns false if creation fails, 
// and leaves the original target intact
//***************************
bool eRenderTarget::Resize(int newWidth, int newHeight) {
	SDL_Texture * newTarget = SDL_CreateTexture( context,
												 SDL_PIXELFORMAT_ARGB8888,		// DEBUG: this format may not work for all images
												 SDL_TEXTUREACCESS_TARGET,
												 newWidth,
												 newHeight );

	if (!newTarget)
		return false;

	// ensure the target can alpha-blend
	SDL_SetTextureBlendMode(newTarget, SDL_BLENDMODE_BLEND);

	if (target == nullptr) {
		target = newTarget;
		return true;
	}

	auto currentTarget = SDL_GetRenderTarget(context);
	if (SDL_SetRenderTarget(context, newTarget) == -1 ||
		SDL_RenderCopy(context, target, nullptr, nullptr) == -1 ||
		SDL_SetRenderTarget(context, currentTarget) == -1) {
		SDL_DestroyTexture(newTarget);
		return false;
	}

	SDL_DestroyTexture(target);
	target = newTarget;
	return true;
}


//***************************
// eRenderTarget::Clear
//***************************
void eRenderTarget::Clear()	{ 
	auto currentTarget = SDL_GetRenderTarget(context);
	SDL_SetRenderTarget(context, target); 
	SDL_SetRenderDrawColor(context, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	SDL_RenderClear(context); 
	SDL_SetRenderTarget(context, currentTarget);
}

//***************************
// eRenderTarget::ClearIfDirty
//***************************
void eRenderTarget::ClearIfDirty(const Uint32 currentTime)	{ 
	if (lastDrawnTime < currentTime) {
		lastDrawnTime = currentTime;
		Clear();
	}
}
