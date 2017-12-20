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

	// DEBUG: eRenderer::defaultOverlayTarget does not need a new SDL_Texture
	defaultOverlayTarget.InitDefault(internal_renderer);

	// initialize debugOverlayTarget for overlay debug info
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	if (!debugOverlayTarget.Init(internal_renderer, viewArea.w, viewArea.h))
		return false;

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("Graphics/Fonts/Alfphabet.ttf", 24);				// FIXME: make this a file-initialized string, not hard-coded

	if (!font)
		return false;

	overlayPool.reserve(MAX_IMAGES);
	overlayPoolInserts.reserve(MAX_IMAGES);
	return true;
}

//***************
// eRenderer::PollEvents
// polls all events in the current window/rendering context
// TODO: allow different callbacks for different events (eg: MOUSE_PRESSED, etc)
//***************
void eRenderer::PollEvents() const {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			game->Stop();
	}
}

//***************
// eRenderer::Shutdown
// close the font and destroy the window
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
	
	point -= target->origin;
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
	for (size_t i = 0; i < points.size(); ++i) {
		eMath::CartesianToIsometric(points[i].x, points[i].y);
		points[i] -= target->origin;
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
	for (size_t i = 0; i < fPoints.size(); ++i) {
		fPoints[i].x -= fPoints[i].z;
		fPoints[i].y -= fPoints[i].z;
		eMath::CartesianToIsometric(fPoints[i].x, fPoints[i].y);
		fPoints[i] -= (eVec3)target->origin;
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
	for (size_t i = 0; i < fPoints.size(); ++i) {
		eMath::CartesianToIsometric(fPoints[i].x, fPoints[i].y);
		fPoints[i] -= target->origin;
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
	SDL_Rect drawRect = {	eMath::NearestInt(rect[0].x - target->origin.x), 
							eMath::NearestInt(rect[0].y - target->origin.y), 
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
void eRenderer::DrawImage(eRenderImage * renderImage) const {
	if (renderImage->targetSrcRect == renderImage->srcRect && !renderImage->srcRects.empty()) {
		renderImage->targetSrcRect = &renderImage->srcRects.front();
	} else {
		renderImage->targetSrcRect = renderImage->srcRect;
	}

//	eVec2 srcRectOrigin((float)renderImage->srcRect->x, (float)renderImage->srcRect->y);
//	eVec2 tgtRectOrigin((float)renderImage->targetSrcRect->x, (float)renderImage->targetSrcRect->y);

	// FIXME(?): account for the offset/size of the smaller srcRect
	// FIXME(!): sometimes the overlap is literally just a line (no width/height), so in those cases IGNORE the draw-order shenanigans
	eVec2 drawPoint = renderImage->origin - currentRenderTarget->origin;
	drawPoint.SnapInt();
	renderImage->dstRect = { (int)drawPoint.x, (int)drawPoint.y, renderImage->targetSrcRect->w, renderImage->targetSrcRect->h };
	SDL_RenderCopy(internal_renderer, renderImage->image->Source(), renderImage->targetSrcRect, &renderImage->dstRect);

	if (!renderImage->srcRects.empty() && renderImage->targetSrcRect != &renderImage->srcRects.back()) {
		renderImage->targetSrcRect++;
	} else {									// the last srcRect is being drawn
		renderImage->targetSrcRect = nullptr;	// setup for the next camera to re-calculate any different srcRects and iterate those (or just for the next frame)
		renderImage->srcRects.clear();		
	}
}

//***************
// eRenderer::RegisterCamera
// registered cameras can have their renderPools modified (add/flush)
// returns true if the camera was not already
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
// returns true if the camera existed
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
// eRenderer::IsAlreadyDrawn
// checks if param renderImage is already assigned
// the the given renderTarget to be drawn this frame
// and updates the renderImage's lastDrawnTime according to eGame::gameTime
// DEBUG: it's quicker to do a linear search of the small drawnTo vector for a eRenderTarget *,
// than it is to search the larger renderPool for a eRenderImage *
//***************
bool eRenderer::CheckDrawnStatus(eRenderTarget * renderTarget, eRenderImage * renderImage) const {
	auto gameTime = game->GetGameTime();
	if (renderImage->lastDrawnTime < gameTime) {
		renderImage->lastDrawnTime = gameTime;
		renderImage->drawnTo.clear();			// first time being drawn this frame
		return false;
	}

	const auto & searchIndex = std::find(renderImage->drawnTo.begin(), renderImage->drawnTo.end(), renderTarget);
	return (searchIndex != renderImage->drawnTo.end());
}

//***************
// eRenderer::AddToCameraRenderPool
// adds param renderImage to param registeredCamera for
// later rendering during Flush (if the camera is registered to *this)
// returns true if param renderImage hasn't already been added to the camera's renderPool
// returns false if it's already in the camera's renderPool
// DEBUG: doesn't check if the camera is genuinely registered, 
// if not, then param renderImage won't be drawn to the rendering context
// and the given camera's pools won't be cleared (as happens during Flush)
// DEBUG: does not draw any eRenderImages on the eCamera::debugRenderTarget
//***************
bool eRenderer::AddToCameraRenderPool(eCamera * registeredCamera, eRenderImage * renderImage) {
	const auto & renderTarget = &registeredCamera->renderTarget;
	if (CheckDrawnStatus(renderTarget, renderImage))
		return false;

	renderImage->drawnTo.emplace_back(renderTarget);
	const auto & renderPool = (renderImage->Owner()->IsStatic() ? &registeredCamera->cameraPool : &registeredCamera->cameraPoolInserts);
	renderPool->emplace_back(renderImage);
	return true;
}

//***************
// eRenderer::AddToOverlayRenderPool
// adds param renderImage to one of the overlayPools for later rendering during Flush
// returns true if param renderImage hasn't already been added to the overlayPool
// returns false if it's already in the overlayPool
// DEBUG: does not draw any eRenderImages on the eRenderer::debugOverlayTarget
//***************
bool eRenderer::AddToOverlayRenderPool(eRenderImage * renderImage) {
	const auto & renderTarget = &defaultOverlayTarget;			
	if (CheckDrawnStatus(renderTarget, renderImage))
		return false;

	renderImage->drawnTo.emplace_back(renderTarget);
	const auto & renderPool = (renderImage->Owner()->IsStatic() ? &overlayPool : &overlayPoolInserts);
	renderPool->emplace_back(renderImage);
	return true;
}

//***************
// eRenderer::TopologicalDrawDepthSort
// assigns draw order priority to the given eRenderImage(s)
// based on their positions relative to the camera
// DEBUG: this is best used on either an entire eRenderer::staticPool/eRenderer::dynamicPool for a frame
// or ONCE for all static geometry in game at startup, followed by adjusting the eRenderImage::priority of dynamic geometry separately
// (starting, for example, with calling this with those items to establish a "localDrawDepth" order amongst them)
//***************
void eRenderer::TopologicalDrawDepthSort(const std::vector<eRenderImage *> & renderImagePool) {
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
void eRenderer::VisitTopologicalNode(eRenderImage * renderImage) {
	if (!renderImage->visited) {
		renderImage->visited = true;
		while (!renderImage->allBehind.empty()) {
			VisitTopologicalNode(renderImage->allBehind.back());
			renderImage->allBehind.pop_back();
		}
		renderImage->priority = (float)globalDrawDepth++;
		renderImage->allBehind.clear();						// FIXME: redundant?
	}
}

//***************
// eRenderer::Flush
//***************
void eRenderer::Flush() {
	for (auto && camera : registeredCameras) {
		FlushCameraPool(camera);
		SetRenderTarget(&camera->renderTarget);
		SDL_RenderCopy(internal_renderer, camera->debugRenderTarget.target, NULL, NULL);		// debug targets are overlays on their counterpart renderTarget
	}

	// transfer cameras (and their debugs') info to the main render texture
	// FIXME: determine a simple sorting order for camera textures, back-to-front
	// possibly use a renderTarget::origin.z value
	SetRenderTarget(&defaultOverlayTarget);
	for (auto && camera : registeredCameras)
		SDL_RenderCopy(internal_renderer, camera->renderTarget.target, NULL, NULL);

	// draw all overlay (HUD/UI/Screen-Space) info now
	FlushOverlayPool();
	SetRenderTarget(&defaultOverlayTarget);
	SDL_RenderCopy(internal_renderer, debugOverlayTarget.target, NULL, NULL);
}

//***************
// eRenderer::FlushCameraPool
// DEBUG: this unstable quicksort may put renderImages at random draw orders if they have equal priority
//***************
void eRenderer::FlushCameraPool(eCamera * registeredCamera) {

	auto & cameraPool = registeredCamera->cameraPool;
	auto & cameraPoolInserts = registeredCamera->cameraPoolInserts;

	// sort the dynamicPool for the scalableTarget
	QuickSort(	cameraPool.data(),
				cameraPool.size(),
				[](auto && a, auto && b) {
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0;
			});

// FREEHILL BEGIN 3d topological sort
	TopologicalDrawDepthSort(cameraPoolInserts);	// assign a "localDrawDepth" priority amongst the cameraPoolInserts
	for (auto & imageToInsert : cameraPoolInserts) {
		bool firstBehind = false;

		// minimize time sent re-sorting static renderImages and just insert the dynamic ones
		for (auto & iter = cameraPool.begin(); iter != cameraPool.end() ; ++iter) {
			if (eCollision::AABBAABBTest(imageToInsert->worldClip, (*iter)->worldClip)) {	
				if (eCollision::IsAABB3DInIsometricFront(imageToInsert->renderBlock, (*iter)->renderBlock)) {
					if (firstBehind) {
						imageToInsert->srcRects.emplace_back( imageToInsert->GetOverlapImageFrame( (*iter)->worldClip ) );

						// insert after what's behind
						if (iter != std::prev(cameraPool.end()))
							cameraPool.emplace(std::next(iter), imageToInsert);
						else
							cameraPool.emplace_back(imageToInsert);
					}
				} else if (!firstBehind) { // imageToInsert is behind *iter, insert before it
					cameraPool.emplace(iter, imageToInsert);
					firstBehind = true;
				}												
			}
		}

		// FIXME: if imageToInsert is never behind anything it never gets inserted
		// FIXME/BUG: weird flicker when moving around in non-cycle areas (except cant stop in a place where the sprite just doesn't draw)
		// ...its possible the srcRects gets a weird/small/shifted rect added to it
		if (!firstBehind)
			cameraPool.emplace_back(imageToInsert);

	}
// FREEHILL END 3d topological sort

	// sets the render target, and scales according to camera zoom
	SetRenderTarget(&registeredCamera->renderTarget);

	// draw to the scalableTarget
	for (auto && renderImage : cameraPool)
		DrawImage(renderImage);

	cameraPool.clear();
	cameraPoolInserts.clear();
}

//***************
// eRenderer::FlushOverlayPool
// DEBUG: this unstable quicksort may put renderImages at random draw orders if they have equal priority
// TODO: mirror FlushCameraPool Topological sort an inserts logic here
//***************
void eRenderer::FlushOverlayPool() {
	// sort the staticPool for the default render target
	QuickSort(	overlayPool.data(),
				overlayPool.size(), 
				[](auto && a, auto && b) { 
					if (a->priority < b->priority) return -1;
					else if (a->priority > b->priority) return 1;
					return 0; 
			});

	// set the render target, and scale to 1.0f
	SetRenderTarget(&defaultOverlayTarget);

	for (auto && renderImage : overlayPool)
		DrawImage(renderImage);

	overlayPool.clear();
	overlayPoolInserts.clear();
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
	SDL_SetRenderTarget(internal_renderer, target->target);
	SDL_RenderSetScale(internal_renderer, target->zoomLevel, target->zoomLevel);
}

//***************
// eRenderer::Show
// updates the window with the rendering context contents
//***************
void eRenderer::Show() const {
	SDL_RenderPresent(internal_renderer);
}


//***************
// eRenderer::GetDefaultOverlayTarget
//***************
eRenderTarget * const eRenderer::GetDefaultOverlayTarget() {
	return &defaultOverlayTarget;
}

//***************
// eRenderer::GetDebugOverlayTarget
//***************
eRenderTarget * const eRenderer::GetDebugOverlayTarget() {
	return &debugOverlayTarget;
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

