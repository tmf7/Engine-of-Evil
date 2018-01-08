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

using namespace evil;

ECLASS_DEFINITION(eGameObject, eCamera)

//***************
// eCamera::Configure
// DEBUG: use this fn instead of eRenderTarget::Init to initialize *this
// DEBUG: automatically registers *this with the eGame->eRenderer object
//***************
void eCamera::Configure(const eVec2 & size, const eVec2 & worldPosition, float zoomLevel, float panSpeed) {
	auto & renderer = game->GetRenderer();
	const auto & context = renderer.GetSDLRenderer();
	this->panSpeed = panSpeed;
	SDL_Point intSize = { eMath::NearestInt(size.x), eMath::NearestInt(size.y) };
	eRenderTarget::Init( context, intSize.x, intSize.y, worldPosition, vec2_one * zoomLevel );
	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);
	renderer.RegisterRenderTarget(this);
}

//***************
// eCamera::Think
// FIXME(?): a different fn should control the camera movement, and any of its restrictions
// because there can be more than one eCamera instance in a game
// SOLUTION(?): derive from a base eCamera class and override Think
//***************
void eCamera::Think() {
	auto & input = game->GetInput();
	const eVec2 oldBoundsCenter = absBounds.Center();		// stay centered on the same point when zooming in/out, it's less jarring

	if (input.KeyPressed(SDL_SCANCODE_EQUALS) || input.GetMouseScroll() > 0)
		ZoomIn();
	else if (input.KeyPressed(SDL_SCANCODE_MINUS) || input.GetMouseScroll() < 0)
		ZoomOut();

	float x = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_D) - input.KeyHeld(SDL_SCANCODE_A));
	float y = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_S) - input.KeyHeld(SDL_SCANCODE_W));
	SetOrigin( origin + (oldBoundsCenter - absBounds.Center()) + (eVec2( x , y ) * game->GetDeltaTime()) );
}

//***************
// eCamera::ZoomIn
// uniformly scales up by zoomSpeed
//***************
void eCamera::ZoomIn() {
	float zoom = scale.x;
	zoom += zoomSpeed;
	if (zoom > maxZoom)
		zoom = maxZoom;

	SetScale(vec2_one * zoom);
}

//***************
// eCamera::ZoomOut
// uniformly scales down by zoomSpeed
//***************
void eCamera::ZoomOut() {
	float zoom = scale.x;
	zoom -= zoomSpeed;
	if (zoom < minZoom)
		zoom = minZoom;

	SetScale(vec2_one * zoom);
}

//***************
// eCamera::SetZoom
//***************
void eCamera::SetZoom(float newZoomLevel) {
	if (newZoomLevel < minZoom)
		newZoomLevel = minZoom;
	else if (newZoomLevel > maxZoom)
		newZoomLevel = maxZoom;

	SetScale(vec2_one * newZoomLevel);
}

//***************
// eCamera::GetZoom
// convenience function to query the uniform eRenderTarget::scale
//***************
float eCamera::GetZoom() const {
	return scale.x;
}

//***************
// eCamera::Moved
// checks if this camera has moved since the last Moved call
// DEBUG: includes zoom and translation
//***************
bool eCamera::Moved() {
	bool moved = ( GetOriginDelta() != vec2_zero || GetScaleDelta() != vec2_zero );
	SetOrigin( origin );
	SetZoom( GetZoom() );
	return moved;
}

//**************
// eCamera::ScreenToWorldPosition
// returns current position of screenPoint over the 2D orthographic game world with respect to this camera's position
//**************
eVec2 eCamera::ScreenToWorldPosition(const eVec2 & screenPoint) const {
	eVec2 worldPoint = (screenPoint / GetZoom()) + absBounds[0];
	eMath::IsometricToCartesian(worldPoint.x, worldPoint.y);
	return worldPoint;
}

//**************
// eCamera::MouseWorldPosition
// returns current position of mouse over the isometric game world with respect to this camera's position
//**************
eVec2 eCamera::MouseWorldPosition() const {
	auto & input = game->GetInput();
	eVec2 screenPoint = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());
	return ScreenToWorldPosition(screenPoint);
}

//***************
// eCamera::AddToRenderPool
// adds param renderImage to either staticPool or dynamicPool,
// which later renders during eRenderer::Flush (if the camera is registered to the eRenderer)
// returns true if param renderImage hasn't already been added to this eCamera's renderPool
// returns false otherwise
// DEBUG: doesn't check if the eCamera is genuinely registered with the eRenderer, 
// if not, then param renderImage won't be drawn to the rendering context
// and this eCamera's pools won't be cleared (as happens during Flush)
//***************
bool eCamera::AddToRenderPool(eRenderImageIsometric * renderImage) {
	if (renderImage->UpdateDrawnStatus(this))
		return false;

	const auto & renderPool = (renderImage->Owner()->IsStatic() ? &staticPool : &dynamicPool);
	renderPool->emplace_back(renderImage);
	return true;
}

//**************
// eCamera::ClearRenderPools
//**************
void eCamera::ClearRenderPools() {
	staticPool.clear();
	dynamicPool.clear();
}

//**************
// eCamera::Flush
// draws all the currently queued eRenderImageIsometric objects to
// this eCamera's eRenderTarget texture, if any
//**************
void eCamera::Flush() {
	if (!visible)
		return;

	QuickSort(	staticPool.data(),
				staticPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

// FREEHILL BEGIN 3d topological sort
	
	// assign a "localDrawDepth" priority amongst the dynamicPool
	eRenderer::TopologicalDrawDepthSort(dynamicPool);

	for (auto & imageToInsert : dynamicPool) {
		bool behindAnotherRenderBlock = false;

		// minimize time spent re-sorting static images and just insert the dynamic ones
		for (auto & iter = staticPool.begin(); iter != staticPool.end(); ++iter) {
			if (eCollision::AABBAABBTest(imageToInsert->GetWorldClip(), (*iter)->GetWorldClip())) {	
				if (!eCollision::IsAABB3DInIsometricFront(imageToInsert->GetRenderBlock(), (*iter)->GetRenderBlock())) {
					behindAnotherRenderBlock = true;
					staticPool.emplace(iter, imageToInsert);
					break;
				}												
			}
		}

		if (!behindAnotherRenderBlock)
			staticPool.emplace_back(imageToInsert);
	}
// FREEHILL END 3d topological sort

	auto & renderer = game->GetRenderer();
	renderer.SetRenderTarget(this);

	for (auto && renderImage : staticPool)
		renderer.DrawImage(renderImage);

	ClearRenderPools();

	renderer.SetRenderTarget(renderer.GetMainRenderTarget());
	SDL_RenderCopy(renderer.GetSDLRenderer(), target, NULL, NULL);
}

//***************
// eCamera::RegisterOverlayCanvas
// registered eCanvases can have their renderPools Flushed
// on top of this eCamera render target
// returns true if the eCanvas was not already
// registered to *this, and could be added
// returns false otherwise
//***************
bool eCamera::RegisterOverlayCanvas(eCanvas * newOverlay) {
	bool addSuccess = (std::find(registeredOverlays.begin(), registeredOverlays.end(), newOverlay) == registeredOverlays.end());
	if (addSuccess)
		registeredOverlays.emplace_back(newOverlay);

	return addSuccess;
}

//***************
// eCamera::UnregisterOverlayCanvas
// returns true if the eCanvas existed
// and was removed from those registered to *this
// returns false otherwise
//***************
bool eCamera::UnregisterOverlayCanvas(eCanvas * overlay) {
	const auto & searchIndex = std::find(registeredOverlays.begin(), registeredOverlays.end(), overlay);
	bool removeSuccess = (searchIndex != registeredOverlays.end());
	if (removeSuccess)
		registeredOverlays.erase(searchIndex);

	return removeSuccess;
}

//***************
// eCamera::UnregisterAllOverlayCanvases
//***************
void eCamera::UnregisterAllOverlayCanvases() {
	registeredOverlays.clear();
}

//***************
// eCamera::NumRegisteredOverlayCanvases
//***************
int eCamera::NumRegisteredOverlayCanvases() const {
	return registeredOverlays.size();
}