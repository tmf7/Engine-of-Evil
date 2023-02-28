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
#ifndef EVIL_RENDER_TARGET_H
#define EVIL_RENDER_TARGET_H

#include "SDL.h"
#include "Vector.h"

//*******************************
//		eRenderTarget
// eRenderer clears this texture
// before drawing to it for the 
// first time each frame, if it's dirty
// DEBUG: similar(?) to Unity's Canvas object
//*******************************
class eRenderTarget {
public:

	friend class eRenderer;

public:

									eRenderTarget() = default;

	void							InitDefault(SDL_Renderer * context, float zoomLevel = 1.0f);
	bool							Init(SDL_Renderer * context, int width, int height, const eVec2 & contextPosition = vec2_zero, float zoomLevel = 1.0f);
	bool							Resize(int newWidth, int newHeight);
	void							ClearIfDirty(const Uint32 currentTime);
	void							Clear();
	void							SetOrigin(const eVec2 & newOrigin)								{ origin = newOrigin; }
	const eVec2 &					GetOrigin() const												{ return origin; }
	void							SetZoom(float newZoomLevel)										{ zoomLevel = newZoomLevel; }
	float							GetZoom() const													{ return zoomLevel; }
	void							ResetZoom()														{ zoomLevel = 1.0f; }
	SDL_Texture * const				GetTarget() const												{ return target; }
	const SDL_Color &				GetClearColor() const											{ return clearColor; }
	void							SetClearColor(const SDL_Color & newClearColor)					{ clearColor = newClearColor; }
	Uint32							GetLastDrawnTime() const										{ return lastDrawnTime; }
	void							SetLastDrawnTime(const Uint32 currentTime)						{ lastDrawnTime = currentTime;}
	bool							IsNull() const													{ return target == nullptr; }


private:

	SDL_Renderer *					context			= nullptr;										// back-pointer to eRenderer-allocated rendering context for target
	SDL_Texture *					target			= nullptr;										// DEBUG: nullptr is the default rendering target for an SDL_Renderer context
	SDL_Color						clearColor		= { 128, 128, 128, SDL_ALPHA_TRANSPARENT };		// clearColor
	Uint32							lastDrawnTime	= 0;											// governs if *this should be cleared on a new frame
	float							zoomLevel		= 1.0f;											// scaling multiplier when rendering to the main context
	eVec2							origin			= vec2_zero;									// position offset when rendering to the main context
};

// utility colors
extern const SDL_Color greyColor_trans;
extern const SDL_Color greyColor_opaque;
extern const SDL_Color blackColor;
extern const SDL_Color greenColor;
extern const SDL_Color redColor;
extern const SDL_Color blueColor;
extern const SDL_Color pinkColor;
extern const SDL_Color lightBlueColor;
extern const SDL_Color yellowColor;
extern const SDL_Color whiteColor;

#endif /* EVIL_RENDER_TARGET_H */