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
// wrapper class for drawing to an SDL_Texture
// attached to a SDL_Renderer context which
// has a user-defined position 
// and axis-aligned bounding box
// DEBUG: eRenderer clears this texture
// before drawing to it for the 
// first time each frame, if it's dirty
// DEBUG: this component does not provide
// a relative offset from its owner's origin,
// so SetOrigin on one affects both equally
//*******************************
class eRenderTarget : public eComponent {

	ECLASS_DECLARATION(eRenderTarget)
	ECOMPONENT_DECLARATION(eRenderTarget)

public:

									eRenderTarget(SDL_Renderer * context, int width, int height, const eVec2 & contextPosition = vec2_zero, const eVec2 & scale = vec2_one);

	void							InitDefault(SDL_Renderer * context, const eVec2 & scale = vec2_one);
	bool							Resize(int newWidth, int newHeight);
	void							ClearIfDirty(const Uint32 currentTime);
	void							Clear();
	void							SetOrigin(const eVec2 & newOrigin);
	void							SetScale(const eVec2 & newScale);
	void							ResetScale();
	const eVec2 &					GetOrigin() const												{ return owner->GetOrigin(); }
	eVec2							GetOriginDelta() const											{ return owner->GetOrigin() - oldOrigin; }
	const eVec2 &					GetScale() const												{ return scale; }
	eVec2							GetScaleDelta() const											{ return scale - oldScale; }
	const eBounds &					AbsBounds() const												{ return absBounds; }
	SDL_Texture * const				GetTargetTexture() const										{ return target; }
	const SDL_Color &				GetClearColor() const											{ return clearColor; }
	void							SetClearColor(const SDL_Color & newClearColor)					{ clearColor = newClearColor; }
	Uint32							GetLastDrawnTime() const										{ return lastDrawnTime; }
	void							SetLastDrawnTime(const Uint32 currentTime)						{ lastDrawnTime = currentTime;}
	int								GetLayer() const												{ return layer; }
	void							SetLayer(int newLayer)											{ layer = newLayer; }
	bool							IsVisible() const												{ return visible; }
	void							SetVisibility(bool newVisibility)								{ visible = newVisibility; }
	bool							IsNull() const													{ return (!isDefault && target == nullptr); }
	bool							Validate();

	virtual void					Update() override;

private:

	void							UpdateBounds();


protected:

	SDL_Renderer *					context			= nullptr;										// back-pointer to eRenderer-allocated rendering context for target
	SDL_Texture *					target			= nullptr;										// DEBUG: nullptr is the default rendering target for an SDL_Renderer context
	SDL_Color						clearColor		= { 128, 128, 128, SDL_ALPHA_TRANSPARENT };		// clearColor
	eBounds							absBounds;														// target size and position within the main rendering context (world-space)
	eVec2							defaultSize;													// allows scale up/down with minimal precision-loss
	eVec2							oldOrigin		= vec2_zero;									// origin as of the last SetOrigin call
	eVec2							scale			= vec2_one;										// scaling multipliers when rendering to the main context
	eVec2							oldScale		= vec2_one;										// scale as of the last SetScale call
	float							lastDrawnTime	= 0.0f;											// governs if *this should be cleared on a new frame
	int								layer			= 0;											// allows high-level draw-order sorting of eRenderTargets during eRenderer::FlushRegisteredRenderTargets
	bool							visible			= true;											// whether or not to copy *this to the main render target during eRenderer::FlushRegisteredRenderTargets
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