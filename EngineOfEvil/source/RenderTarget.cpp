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
#include "RenderTarget.h"

using namespace evil;

ECLASS_DEFINITION(eComponent, eRenderTarget)
ECOMPONENT_DEFINITION(eRenderTarget)

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
// the default render target for the context
// does not need a position, size, or SDL_Texture
//***************************
 void eRenderTarget::InitDefault(SDL_Renderer * context, const eVec2 & scale) {
	this->context = context;
	this->scale = scale;
	isDefault = true;
 }

//***************************
// eRenderTarget::eRenderTarget
// creates a new SDL_Texture within the given param context
// returns true on success, false on failure
//***************************
eRenderTarget::eRenderTarget(eGameObject * owner, SDL_Renderer * context, int width, int height, const eVec2 & scale)
	: width(width),
	  height(height) {
	this->owner = owner;
	InitDefault(context, scale);
	isDefault = false;

	target = SDL_CreateTexture( context,
								SDL_PIXELFORMAT_ARGB8888,						// DEBUG: this format may not work for all images
								SDL_TEXTUREACCESS_TARGET,
								width,
								height );

	if (IsNull()) {
		EVIL_ERROR_LOG.LogError("Failed to allocate new eRenderTarget texture.", __FILE__, __LINE__);
		return;
	}

	// ensure the target can alpha-blend
	SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
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
		EVIL_ERROR_LOG.LogError("Failed to allocate new eRenderTarget texture.", __FILE__, __LINE__);
		return false;
	}

	SDL_DestroyTexture(target);
	target = newTarget;
	width = newWidth;
	height = newHeight;
	return true;
}

//***************
// eRenderTarget::Validate
// attempts to validate if IsNull is true
// returns false if *this can't be drawn to
// returns true otherwise
//***************
bool eRenderTarget::Validate() {
	return ( !IsNull() || Resize( width, height ) );
}

//***************************
// eRenderTarget::Clear
//***************************
void eRenderTarget::Clear()	{ 
	if ( !Validate() )
		return;

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

//***************************
// eRenderTarget::GetOrigin
// if the eGameObject owner is nullptr (as for the main/default render target)
// returns vec2_zero, otherwise
// returns the owner's orthoOrigin
//***************************
const eVec2 & eRenderTarget::GetOrigin() const {
	return ( owner == nullptr ? vec2_zero : owner->GetOrigin() );
}
