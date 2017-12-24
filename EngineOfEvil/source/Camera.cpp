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

//***************
// eCamera::Init
//***************
void eCamera::Init(const eVec2 & size, const eVec2 & worldPosition, float zoomLevel, float panSpeed) {
	this->panSpeed = panSpeed;
	defaultSize = size;
	const auto & context = game->GetRenderer().GetSDLRenderer();
	SDL_Point intSize = { eMath::NearestInt(defaultSize.x), eMath::NearestInt(defaultSize.y) };
	renderTarget.Init( context, intSize.x, intSize.y, worldPosition, zoomLevel );
	debugRenderTarget.Init( context, intSize.x, intSize.y, worldPosition, zoomLevel );
	UpdateZoom();
	cameraPool.reserve(MAX_IMAGES);
	cameraPoolInserts.reserve(MAX_IMAGES);
}

//***************
// eCamera::Think
// FIXME(?): a different fn should control the camera movement, and any of its restrictions
// because there can be more than one eCamera instance in a game
// SOLUTION(?): derive from a base eCamera class and override Think
//***************
void eCamera::Think() {
	auto & input = game->GetInput();
	float oldZoomLevel = renderTarget.GetZoom();
	const eVec2 oldOrigin = renderTarget.GetOrigin();
	const eVec2 oldBoundsCenter = absBounds.Center();		// stay centered on the same point when zooming in/out, it's less jarring

	if (input.KeyPressed(SDL_SCANCODE_EQUALS) || input.GetMouseScroll() > 0)
		ZoomIn();
	else if (input.KeyPressed(SDL_SCANCODE_MINUS) || input.GetMouseScroll() < 0)
		ZoomOut();

	const eVec2 zoomedBoundsCenter = absBounds.Center();

	if (input.KeyHeld(SDL_SCANCODE_SPACE)) {
//		eVec2 snapFocus = map->GetEntity(0)->CollisionModel().Center();		// FIXME: 0th eEntity should not be the default thing to snap focus to
//		eMath::CartesianToIsometric(snapFocus.x, snapFocus.y);
//		SetOrigin(snapFocus);
	} else {
		float x = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_D) - input.KeyHeld(SDL_SCANCODE_A));
		float y = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_S) - input.KeyHeld(SDL_SCANCODE_W));
		float deltaSec = (float)game->GetDeltaTime() / 1000.0f;
		deltaSec = deltaSec >= 1.0f ? 0.99f : deltaSec;
		SetOrigin( oldOrigin + (oldBoundsCenter - zoomedBoundsCenter) + (eVec2( x , y ) / (1.0f - deltaSec )) );
	}

	moved = (renderTarget.GetZoom() != oldZoomLevel || renderTarget.GetOrigin() != oldOrigin);
}

//***************
// eCamera::UpdateZoom
// ensures the collisionModel bounds
// is syncronized with the zoomLevel
//***************
void eCamera::UpdateZoom() {
	eVec2 zoomSize = defaultSize / renderTarget.GetZoom();
	absBounds = eBounds(vec2_zero, zoomSize) + renderTarget.GetOrigin();
}

//***************
// eCamera::SetSize
// resizes both eRenderTargets 
// by copying their data to targets
// and destroying the old ones
//***************
void eCamera::SetSize(const eVec2 & newSize) {
	defaultSize = newSize;
	UpdateZoom();
	SDL_Point intSize = { eMath::NearestInt(defaultSize.x), eMath::NearestInt(defaultSize.y) };
	renderTarget.Resize( intSize.x, intSize.y );
	debugRenderTarget.Resize( intSize.x, intSize.y );
}

//***************
// eCamera::ZoomIn
// scales both render targets' data
//***************
void eCamera::ZoomIn() {
	float zoom = renderTarget.GetZoom();
	zoom += zoomSpeed;
	if (zoom > maxZoom)
		zoom = maxZoom;

	renderTarget.SetZoom(zoom);
	debugRenderTarget.SetZoom(zoom);
	UpdateZoom();
}

//***************
// eCamera::ZoomOut
// de-scales both render targets' data
//***************
void eCamera::ZoomOut() {
	float zoom = renderTarget.GetZoom();
	zoom -= zoomSpeed;
	if (zoom < minZoom)
		zoom = minZoom;

	renderTarget.SetZoom(zoom);
	debugRenderTarget.SetZoom(zoom);
	UpdateZoom();
}

//***************
// eCamera::ResetZoom
// scales both render targets' data back to 1.0f
//***************
void eCamera::ResetZoom() {
	renderTarget.ResetZoom();
	debugRenderTarget.ResetZoom();
	UpdateZoom();
}

//***************
// eCamera::GetZoom
// convenience function to query the renderTarget zoomLevel
// DEBUG: which is equal to the debugRenderTarget zoomLevel
//***************
float eCamera::GetZoom() const {
	return renderTarget.GetZoom();
}

//***************
// eCamera::SetOrigin
// keeps the renderTarget and 
// debugRenderTarget positions synchronized
//***************
void eCamera::SetOrigin(const eVec2 & newOrigin) {
	renderTarget.SetOrigin(newOrigin);
	debugRenderTarget.SetOrigin(newOrigin);
	UpdateZoom();
}

//***************
// eCamera::GetOrigin
// convenience fn to query renderTarget origin
// DEBUG: with is equal to the debugRenderTarget origin
//***************
const eVec2 & eCamera::GetOrigin() const {
	return renderTarget.GetOrigin();
}

//***************
// eCamera::Moved
// DEBUG: includes zoom and translation
//***************
bool eCamera::Moved() const {
	return moved;
}

//***************
// eCamera::AbsBounds
// returns the current worldPosition and dimensions of *this
//***************
const eBounds & eCamera::AbsBounds() const {
	return absBounds;
}

//**************
// eCamera::ScreenToWorldPosition
// returns current position of screenPoint over the 2D orthographic game world
//**************
eVec2 eCamera::ScreenToWorldPosition(const eVec2 & screenPoint) const {
	eVec2 worldPoint = (screenPoint / renderTarget.GetZoom()) + absBounds[0];
	eMath::IsometricToCartesian(worldPoint.x, worldPoint.y);
	return worldPoint;
}

//**************
// eCamera::MouseWorldPosition
// returns current position of mouse over the isometric game world
//**************
eVec2 eCamera::MouseWorldPosition() const {
	auto & input = game->GetInput();
	eVec2 screenPoint = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());
	return ScreenToWorldPosition(screenPoint);
}

//***************
// eCamera::GetRenderTarget
//***************
eRenderTarget * const eCamera::GetRenderTarget() {
	return &renderTarget;
}

//***************
// eCamera::GetDebugRenderTarget
//***************
eRenderTarget * const eCamera::GetDebugRenderTarget() {
	return &debugRenderTarget;
}

