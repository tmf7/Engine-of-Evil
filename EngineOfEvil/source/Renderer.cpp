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

using namespace evil;

ECLASS_DEFINITION(eClass, eRenderer)

int eRenderer::globalDrawDepth	= 0;

//***************
// eRenderer::Init
// initialize the window, its rendering context, and a default font
//***************
bool eRenderer::Init(const char * name, int windowWidth, int windowHeight) {
	window = SDL_CreateWindow( name, 
							   SDL_WINDOWPOS_UNDEFINED, 
							   SDL_WINDOWPOS_UNDEFINED, 
							   windowWidth, 
							   windowHeight, 
							   SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );

	if (!window)
		return false;

	// DEBUG: SDL_RENDERER_TARGETTEXTURE allows rendering to SDL_Textures
	internal_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	if (!internal_renderer)
		return false;

	// enable linear anti-aliasing for the renderer context
//	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear" , SDL_HINT_OVERRIDE);

	// DEBUG: eRenderer::mainRenderTarget does not need a new SDL_Texture
	mainRenderTarget.InitDefault(internal_renderer);

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("Graphics/Fonts/Alfphabet.ttf", 24);				// FIXME: make this a file-initialized string, not hard-coded

	if (!font)
		return false;

	return true;
}

//***************
// eRenderer::Shutdown
// close the font and destroy the window
// which also destroys all associated eRenderTargets
//***************
void eRenderer::Shutdown() const {
	if (font)
		TTF_CloseFont(font);

	TTF_Quit();

	if (internal_renderer)
		SDL_DestroyRenderer(internal_renderer);

	if (window)
		SDL_DestroyWindow(window);
}

//***************
// eRenderer::DrawOutlineText
// param constText caches the text image to accelerate redraw
// TODO: this is a debug test, make an actual text rendering class
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawOutlineText(eRenderTarget * target, const char * text, eVec2 & point, const SDL_Color & color, bool constText) {
	SDL_Texture * renderedText = NULL;
	if (constText) {
		// check if the image already exists, if not then load it and set result
		std::shared_ptr<eImage> result;
		game->GetImageManager().LoadAndGetConstantText(font, text, color, result);
		renderedText = result->Source();
	} else {
		SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, color);
		if (surfaceText == NULL)
			return;

		renderedText = SDL_CreateTextureFromSurface(internal_renderer, surfaceText);
		SDL_FreeSurface(surfaceText);
		if (renderedText == NULL)
			return;

		SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);
	}
	
	point -= target->GetOrigin();
	point.SnapInt();
	SDL_Rect dstRect = { (int)point.x, (int)point.y, 0, 0 };
	SDL_QueryTexture(renderedText, NULL, NULL, &dstRect.w, &dstRect.h);
	SetRenderTarget(target);
	SDL_RenderCopy(internal_renderer, renderedText, nullptr, &dstRect);

	if (!constText)
		SDL_DestroyTexture(renderedText);
}

//***************
// eRenderer::DrawLines
// draws points.size()-1 lines from front to back
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawLines(eRenderTarget * target, const SDL_Color & color, std::vector<eVec2>  points) {
	std::vector<SDL_Point> iPoints;
	iPoints.reserve(points.size());
	for (std::size_t i = 0; i < points.size(); ++i) {
		eMath::CartesianToIsometric(points[i].x, points[i].y);
		points[i] -= target->GetOrigin();
		points[i].SnapInt();
		iPoints[i] = { (int)points[i].x, (int)points[i].y };
	}

	SetRenderTarget(target);	
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLines(internal_renderer, iPoints.data(), iPoints.size());
}

//***************
// eRenderer::DrawIsometricPrism
// converts the given rect into an isomectric box
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawIsometricPrism(eRenderTarget * target, const SDL_Color & color, const eBounds3D & rect) {
	std::array<eVec3, 10> fPoints;
	rect.ToPoints(fPoints.data());
	
	// shift and insert points necessary to close the bottom and top
	for (int i = 8; i > 4; --i)
		fPoints[i] = fPoints[i - 1];
	
	fPoints[9] = fPoints[4];
	fPoints[4] = fPoints[0];

	// convert to isometric rhombus
	// and translate with camera
	std::array<SDL_Point, 10> iPoints;
	for (std::size_t i = 0; i < fPoints.size(); ++i) {
		fPoints[i].x -= fPoints[i].z;
		fPoints[i].y -= fPoints[i].z;
		eMath::CartesianToIsometric(fPoints[i].x, fPoints[i].y);
		fPoints[i] -= (eVec3)target->GetOrigin();
		fPoints[i].SnapInt();
		iPoints[i] = { (int)fPoints[i].x, (int)fPoints[i].y };
	}

	SetRenderTarget(target);
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLines(internal_renderer, iPoints.data(), iPoints.size());

	std::array<SDL_Point, 2> verticals;
	verticals[0] = iPoints[6];
	verticals[1] = iPoints[1];
	SDL_RenderDrawLines(internal_renderer, verticals.data(), verticals.size());

	verticals[0] = iPoints[7];
	verticals[1] = iPoints[2];
	SDL_RenderDrawLines(internal_renderer, verticals.data(), verticals.size());

	verticals[0] = iPoints[8];
	verticals[1] = iPoints[3];
	SDL_RenderDrawLines(internal_renderer, verticals.data(), verticals.size());
}

//***************
// eRenderer::DrawIsometricRect
// converts the given rect into an isomectric box
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawIsometricRect(eRenderTarget * target, const SDL_Color & color, const eBounds & rect) {
	std::array<eVec2, 5> fPoints;
	rect.ToPoints(fPoints.data());
	fPoints[4] = fPoints[0];

	// convert to isometric rhombus
	// and translate with camera
	std::array<SDL_Point, 5> iPoints;
	for (std::size_t i = 0; i < fPoints.size(); ++i) {
		eMath::CartesianToIsometric(fPoints[i].x, fPoints[i].y);
		fPoints[i] -= target->GetOrigin();
		fPoints[i].SnapInt();
		iPoints[i] = { (int)fPoints[i].x, (int)fPoints[i].y };
	}

	SetRenderTarget(target);
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLines(internal_renderer, iPoints.data(), iPoints.size());
}

//***************
// eRenderer::DrawCartesianRect
// fill draws the rect solid
// dynamic moves and scales with the camera
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawCartesianRect(eRenderTarget * target, const SDL_Color & color, const eBounds & rect, bool fill) {
	const auto & targetOrigin = target->GetOrigin();
	SDL_Rect drawRect = {	eMath::NearestInt(rect[0].x - targetOrigin.x), 
							eMath::NearestInt(rect[0].y - targetOrigin.y), 
							eMath::NearestInt(rect.Width()), 
							eMath::NearestInt(rect.Height()) };

	SetRenderTarget(target);
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);
	fill ? SDL_RenderFillRect(internal_renderer, &drawRect)
		: SDL_RenderDrawRect(internal_renderer, &drawRect);
}

//***************
// eRenderer::DrawImage
// DEBUG: immediatly draws to the currently assigned render target
//***************
void eRenderer::DrawImage(eRenderImageBase * renderImage) const {
	eVec2 drawPoint = renderImage->origin - currentRenderTarget->GetOrigin();
	drawPoint.SnapInt();
	SDL_Rect dstRect = { (int)drawPoint.x, 
						 (int)drawPoint.y, 
						 eMath::NearestInt( (float)renderImage->srcRect->w * renderImage->scale.x ),
						 eMath::NearestInt( (float)renderImage->srcRect->h * renderImage->scale.y ) };
	SDL_RenderCopy(internal_renderer, renderImage->image->Source(), renderImage->srcRect, &dstRect);
}

//***************
// eRenderer::RegisterCamera
// registered eCameras can have their renderPools Flushed
// directly to the main render target
// returns true if the eCamera was not already
// registered to *this, and could be added
// returns false otherwise
//***************
bool eRenderer::RegisterCamera(eCamera * newCamera) {
	bool addSuccess = (std::find(registeredCameras.begin(), registeredCameras.end(), newCamera) == registeredCameras.end());
	if (addSuccess)
		registeredCameras.emplace_back(newCamera);

	return addSuccess;
}

//***************
// eRenderer::UnregisterCamera
// returns true if the eCamera existed
// and was removed from those registered to *this
// returns false otherwise
//***************
bool eRenderer::UnregisterCamera(eCamera * camera) {
	const auto & searchIndex = std::find(registeredCameras.begin(), registeredCameras.end(), camera);
	bool removeSuccess = (searchIndex != registeredCameras.end());
	if (removeSuccess)
		registeredCameras.erase(searchIndex);

	return removeSuccess;
}

//***************
// eRenderer::UnregisterAllCameras
//***************
void eRenderer::UnregisterAllCameras() {
	registeredCameras.clear();
}

//***************
// eRenderer::NumRegisteredCameras
//***************
int eRenderer::NumRegisteredCameras() const {
	return registeredCameras.size();
}

//***************
// eRenderer::RegisterOverlayCanvas
// registered overlay eCanvases can have their renderPools Flushed
// directly to the main render target
// returns true if the eCanvas was not already
// registered to *this, and could be added
// returns false otherwise
//***************
bool eRenderer::RegisterOverlayCanvas(eCanvas * newCanvas) {
	bool addSuccess = (std::find(registeredOverlayCanvases.begin(), registeredOverlayCanvases.end(), newCanvas) == registeredOverlayCanvases.end());
	if (addSuccess)
		registeredOverlayCanvases.emplace_back(newCanvas);

	return addSuccess;
}

//***************
// eRenderer::UnregisterOverlayCanvas
// returns true if the eCanvas existed
// and was removed from those registered to *this
// returns false otherwise
//***************
bool eRenderer::UnregisterOverlayCanvas(eCanvas * canvas) {
	const auto & searchIndex = std::find(registeredOverlayCanvases.begin(), registeredOverlayCanvases.end(), canvas);
	bool removeSuccess = (searchIndex != registeredOverlayCanvases.end());
	if (removeSuccess)
		registeredOverlayCanvases.erase(searchIndex);

	return removeSuccess;
}

//***************
// eRenderer::UnregisterAllOverlayCanvases
//***************
void eRenderer::UnregisterAllOverlayCanvases() {
	registeredOverlayCanvases.clear();
}

//***************
// eRenderer::NumRegisteredOverlayCanvases
//***************
int eRenderer::NumRegisteredOverlayCanvases() const {
	return registeredOverlayCanvases.size();
}

//***************
// eRenderer::TopologicalDrawDepthSort
// assigns draw order priority to the given eRenderImageIsometric(s)
// based on their positions relative to the camera
// DEBUG: this is best used on either an entire eRenderer::staticPool/eRenderer::dynamicPool for a frame
// or ONCE for all static geometry in game at startup, followed by adjusting the eRenderImageBase::priority of dynamic geometry separately
// (starting, for example, with calling this with those items to establish a "localDrawDepth" order amongst them)
//***************
void eRenderer::TopologicalDrawDepthSort(const std::vector<eRenderImageIsometric *> & renderImagePool) {
	for (auto & self : renderImagePool) {
		auto & selfClip = self->worldClip;

		for (auto & other : renderImagePool) {
			auto & otherClip = other->worldClip;

			if (other != self && 
				eCollision::AABBAABBTest(selfClip, otherClip) && 
				eCollision::IsAABB3DInIsometricFront(self->renderBlock, other->renderBlock))
				self->allBehind.emplace_back(other);
		}
		self->visited = false;
	}

	globalDrawDepth = 0;
	for (auto & renderImage : renderImagePool)
		VisitTopologicalNode(renderImage);
}

//***************
// eRenderer::VisitTopologicalNode
//***************
void eRenderer::VisitTopologicalNode(eRenderImageIsometric * renderImage) {
	if (!renderImage->visited) {
		renderImage->visited = true;
		while (!renderImage->allBehind.empty()) {
			VisitTopologicalNode(renderImage->allBehind.back());
			renderImage->allBehind.pop_back();
		}
		renderImage->priority = (float)globalDrawDepth++;
	}
}

//***************
// eRenderer::Flush
// calls flush on all registered eCameras and eCanvases that have
// eRenderImageBase-derived objects queued to be drawn
// then copys each registered eRenderTarget to the mainRenderTarget
// according to its type (overlay|world-space) and proximity to the viewer
//***************
void eRenderer::Flush() {

	// TODO: any eCanvas renderTargets here are automatically overlays
	// because camera overlays are registered to them,
	// and world-space canvases are in the tilemap
	// SO: sort registeredTargets to put eCanvases first (by layer)
	// then eCameras (by layer)

	for (auto && target : registeredTargets)
		target->Flush();
}

//***************
// eRenderer::SetRenderTarget
// switches the current rendering target
// and clears it (using its clear color)
// if this is its first use this frame
//***************
void eRenderer::SetRenderTarget(eRenderTarget * target) {
	target->ClearIfDirty(game->GetGameTime());
	currentRenderTarget = target;
	SDL_SetRenderTarget(internal_renderer, target->GetTargetTexture());
	const auto & targetScale = target->GetScale();
	SDL_RenderSetScale(internal_renderer, targetScale.x, targetScale.y);
}

//***************
// eRenderer::Show
// updates the window with the rendering context contents
//***************
void eRenderer::Show() const {
	SDL_RenderPresent(internal_renderer);
}

//***************
// eRenderer::GetMainRenderTarget
//***************
eRenderTarget * const eRenderer::GetMainRenderTarget() {
	return &mainRenderTarget;
}

//***************
// eRenderer::ViewArea
// returns the dimensions of the current rendering target
//***************
SDL_Rect eRenderer::ViewArea() const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	return viewArea;
}

//***************
// eRenderer::GetRenderer
// returns the current SDL rendering context
//***************
SDL_Renderer * const eRenderer::GetSDLRenderer() const {
	return internal_renderer;
}

//*****************
// eRenderer::GetWindow
// returns the window the rendering context is registered to
//*****************
SDL_Window * const eRenderer::GetWindow() const {
	return window;
}

