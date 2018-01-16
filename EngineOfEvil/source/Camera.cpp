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
#include "RenderImageIsometric.h"
#include "CollisionModel.h"

using namespace evil;

ECLASS_DEFINITION(eGameObject, eCamera)

//***************
// eCamera::Init
// DEBUG: automatically registers *this with the eGame->eRenderer object
// FIXME: a user may neglect to call Init, leaving collisionModel undefined
// SOLUTION: check for bool initialized or nullptr collisionModel in fns that use collisionModel
// SOLUTION: do a local GetComponent<> always for such things instead (ie: in all eGameObject-derived classes)
//***************
void eCamera::Init(eMap * onMap, const eVec2 & size, const eVec2 & worldPosition, float zoomLevel, float panSpeed) {
	this->panSpeed = panSpeed;
	map = onMap;
	SetOrigin(worldPosition);
	staticPool.reserve(MAX_IMAGES);
	dynamicPool.reserve(MAX_IMAGES);

	// DEBUG: leaves this eCollisionModel with active == false so no collision detection occurs
	AddComponent<eCollisionModel>(this, eBounds(-size * 0.5f, size * 0.5f));
	collisionModel = GetComponent<eCollisionModel>();
	game->GetRenderer().RegisterCamera(this);
}

//***************
// eCamera::Think
// FIXME/TODO: a different fn should control the camera movement, and any of its restrictions
// because there can be more than one eCamera instance in a game
// FIXME(~): stay centered on the same origin when zooming (should be fixed already)
//***************
void eCamera::Think() {
	moved = false;

	auto & input = game->GetInput();
	if (input.KeyPressed(SDL_SCANCODE_EQUALS) || input.GetMouseScroll() > 0)
		ZoomIn();
	else if (input.KeyPressed(SDL_SCANCODE_MINUS) || input.GetMouseScroll() < 0)
		ZoomOut();

	float x = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_D) - input.KeyHeld(SDL_SCANCODE_A));
	float y = panSpeed * (float)(input.KeyHeld(SDL_SCANCODE_S) - input.KeyHeld(SDL_SCANCODE_W));

	const eVec2 panAmount = eVec2( x, y ) * game->GetDeltaTime();
	if ( panAmount != vec2_zero ) {
		SetOrigin( GetOrigin() + panAmount );
		moved = true;
	}
}

//***************
// eCamera::Resize
// resizing an eCamera makes more|less of the gameworld visible
//***************
void eCamera::Resize(const eVec2 & newSize) {
	collisionModel->SetLocalBounds( eBounds( -newSize * 0.5f, newSize * 0.5f ) );
	moved = true;
}

//***************
// eCamera::ZoomIn
// decreases camera viewport size
// effectively scaling up a smaller visible
// portion of the gameworld
//***************
void eCamera::ZoomIn() {
	zoom += zoomSpeed;
	if (zoom > maxZoom)
		zoom = maxZoom;

	Resize( defaultSize / zoom );
}

//***************
// eCamera::ZoomOut
// increases camera viewport size
// effectively scaling up a smaller visible
// portion of the gameworld
//***************
void eCamera::ZoomOut() {
	zoom -= zoomSpeed;
	if (zoom < minZoom)
		zoom = minZoom;

	Resize( defaultSize / zoom );
}

//***************
// eCamera::SetZoom
// sets the zoom within the range [minZoom, maxZoom]
// see also: ZoomIn, ZoomOut
//***************
void eCamera::SetZoom(float newZoomLevel) {
	if (newZoomLevel < minZoom)
		newZoomLevel = minZoom;
	else if (newZoomLevel > maxZoom)
		newZoomLevel = maxZoom;

	zoom = newZoomLevel;
	Resize( defaultSize / zoom );
}

//***************
// eCamera::ResetZoom
// sets the zoom within the range [minZoom, maxZoom]
// see also: ZoomIn, ZoomOut
//***************
void eCamera::ResetZoom() {
	zoom = 1.0f;
	Resize( defaultSize / zoom );
}

//***************
// eCamera::GetZoom
//***************
float eCamera::GetZoom() const {
	return zoom;
}

//***************
// eCamera::Moved
// DEBUG: includes changes in zoom and translation
//***************
bool eCamera::Moved() const {
	return moved;
}

//***************
// eCamera::AbsBounds
//***************
const eBounds & eCamera::AbsBounds() const {
	return collisionModel->AbsBounds();
}

//**************
// eCamera::ScreenToWorldPosition
// returns current position of screenPoint over the 2D orthographic game world with respect to this camera's position
//**************
eVec2 eCamera::ScreenToWorldPosition(const eVec2 & screenPoint) const {
	eVec2 worldPoint = (screenPoint / zoom) + collisionModel->AbsBounds()[0];
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
	if (!renderImage->AddDrawnToItem(this))
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
// draws the currently gameworld within this eCamera's bounds,
// and any registered overlay eCanvases
//**************
void eCamera::Flush() {

	// draw the visible gameworld
	QuickSort(	staticPool.data(),
				staticPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

// FREEHILL BEGIN 3d topological sort
	
	// assign a "localDrawDepth" priority
	// to avoid multiple passes over the dynamicPool
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

	// scale camera contents based on its size relative to the window
	// DEBUG: this also adjusts x/y image coordinates accordingly
	// TODO: give a camera its own viewportRect and add that into the scale calculation
	auto & renderer = game->GetRenderer();
	const SDL_Rect windowBounds = renderer.ViewArea();
	const eBounds & cameraBounds = collisionModel->LocalBounds();
	const eVec2 cameraScale( (float)windowBounds.w / cameraBounds.Width(), 
							 (float)windowBounds.h / cameraBounds.Height() );

	auto mainRenderTarget = renderer.GetMainRenderTarget();
	mainRenderTarget->SetScale(cameraScale);
	renderer.SetRenderTarget(mainRenderTarget);

	// draw calls
	const auto & cameraOffset = GetOrigin();
	for (auto && renderImage : staticPool)
		renderer.DrawImage(renderImage, cameraOffset);

	ClearRenderPools();

	// registered eCanvas orders may haved shifted between frames
	// or a new eCanvas may have been registered
	QuickSort(	registeredOverlays.data(),
				registeredOverlays.size(),
				[](auto && a, auto && b) {
					const int aLayer = a->GetWorldLayer();
					const int bLayer = b->GetWorldLayer();

					if (aLayer < bLayer) return -1;
					else if (aLayer > bLayer) return 1;
					return 0;
			});

	// draw calls
	for (auto && canvas : registeredOverlays)
		canvas->Flush();

	mainRenderTarget->ResetScale();
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