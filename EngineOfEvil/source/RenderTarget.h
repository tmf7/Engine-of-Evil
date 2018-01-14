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
#include "Bounds.h"
#include "Component.h"

namespace evil {

//*******************************
//		eRenderTarget
// wrapper class for drawing to an 
// SDL_TEXTUREACCESS_TARGET type
// SDL_Texture not handled by eImageManager.
// As an eComponent, its lifetime is instead 
// managed by its eGameObject owner.
// DEBUG: eRenderer clears this texture
// before drawing to it for the 
// first time each frame.
//*******************************
class eRenderTarget : public eComponent {

	ECLASS_DECLARATION(eRenderTarget)
	ECOMPONENT_DECLARATION(eRenderTarget)

public:

	virtual						   ~eRenderTarget();
									eRenderTarget(eGameObject * owner, SDL_Renderer * context, int width, int height, const eVec2 & scale = vec2_one);

	void							InitDefault(SDL_Renderer * context, const eVec2 & scale = vec2_one);
	bool							Resize(int newWidth, int newHeight);
	void							ClearIfDirty(const Uint32 currentTime);
	void							Clear();
	const eVec2 &					GetOrigin() const;
	int								GetWidth() const												{ return width; }
	int								GetHeight() const												{ return height; }
	void							SetScale(const eVec2 & newScale)								{ scale = newScale; }
	void							ResetScale()													{ scale = vec2_one; }
	const eVec2 &					GetScale() const												{ return scale; }
	SDL_Texture * const				GetTargetTexture() const										{ return target; }
	const SDL_Color &				GetClearColor() const											{ return clearColor; }
	void							SetClearColor(const SDL_Color & newClearColor)					{ clearColor = newClearColor; }
	Uint32							GetLastDrawnTime() const										{ return lastDrawnTime; }
	void							SetLastDrawnTime(const Uint32 currentTime)						{ lastDrawnTime = currentTime;}
	bool							IsNull() const													{ return (!isDefault && target == nullptr); }
	bool							IsDefaultTarget() const											{ return isDefault; }
	bool							Validate();

protected:

	SDL_Renderer *					context			= nullptr;										// back-pointer to eRenderer-allocated rendering context for target
	SDL_Texture *					target			= nullptr;										// DEBUG: nullptr is the default rendering target for an SDL_Renderer context
	SDL_Color						clearColor		= { 128, 128, 128, SDL_ALPHA_TRANSPARENT };		// clearColor
	eVec2							scale			= vec2_one;										// scaling multipliers when rendering to the main context
	float							lastDrawnTime	= 0.0f;											// governs if *this should be cleared on a new frame
	int								width			= 0;
	int								height			= 0;
	bool							isDefault		= false;										// if this is the default render target for the given context, where targets should be nullptr
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

}      /* evil */

#endif /* EVIL_RENDER_TARGET_H */