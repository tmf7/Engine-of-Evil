#include "Renderer.h"
#include "Game.h"

const SDL_Color clearColor		= { 128, 128, 128, SDL_ALPHA_OPAQUE };
const SDL_Color blackColor		= { 0, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color greyColor_trans = { 0, 0, 0, 64 };
const SDL_Color greenColor		= { 0, 255, 0, 255 };
const SDL_Color redColor		= { 255, 0, 0, 255 };

//***************
// eRenderer::Init
// initialize the window, its rendering context, and a default font
//***************
bool eRenderer::Init() {
	window = SDL_CreateWindow(	"Evil", 
								SDL_WINDOWPOS_UNDEFINED, 
								SDL_WINDOWPOS_UNDEFINED, 
								800, 
								600, 
								SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if (!window)
		return false;

	// DEBUG: TARGETTEXTURE is used to read pixel data from SDL_Textures
	internal_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	if (!internal_renderer)
		return false;

	// enable linear anti-aliasing for the renderer context
//	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear" , SDL_HINT_OVERRIDE);

	if (SDL_SetRenderDrawColor(internal_renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a) == -1)	// opaque grey
		return false;

	// initialize scalableTarget for scaling textures using the camera
	// independent of any static overlay (gui/HUD)
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	scalableTarget = SDL_CreateTexture(internal_renderer,
									   SDL_PIXELFORMAT_ARGB8888,		// DEBUG: this format may not work for all images
									   SDL_TEXTUREACCESS_TARGET,
									   viewArea.w,
									   viewArea.h);

	if (!scalableTarget)
		return false;

	// ensure the scalableTarget can alhpa-blend
	SDL_SetTextureBlendMode(scalableTarget, SDL_BLENDMODE_BLEND);

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		return false;

	return true;
}

//***************
// eRenderer::Free
// close the font and destroy the window
//***************
void eRenderer::Free() const {
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
// Draws the given string on the screen using location and color
// if constText is true then the text is considered unchanging and the
// text's image gets cached for later reuse
// dynamic == true draws to the scalableTarget
// dynamic == false draws to the default render target
// DEBUG: converts the input point float data to integer values
// TODO: potentially scale, rotate, and translate the text for things other than HUD/Toolbox text
// TODO: does not currently accomodate camera translation
//***************
void eRenderer::DrawOutlineText(const char * text, const eVec2 & point, const SDL_Color & color, bool constText, bool dynamic) {
	SDL_Texture * renderedText = NULL;
	std::shared_ptr<eImage> result;
	if (constText) {
		// check if the image already exists, if not then load it and set result
		game.GetImageManager().LoadConstantText(font, text, color, result);
		renderedText = result->Source();
	} else {
		SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, color);
		if (surfaceText == NULL)
			return;

		renderedText = SDL_CreateTextureFromSurface(internal_renderer, surfaceText);
		SDL_FreeSurface(surfaceText);
		if (renderedText == NULL)
			return;

		// FIXME/BUG(?): flushing the renderPool SHOULD destroy this block of memory 
		// as that is the only thing with a copy of this std::shared_ptr<eImage>
		// (otherwise a heap overflow/segfault will eventually occur if lots of non-const text is drawn)
		result = std::make_shared<eImage>(renderedText, "non_const_text", INVALID_ID);
		SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);
	}

	SDL_Rect dstRect { (int)point.x , (int)point.y };
	SDL_QueryTexture(renderedText, NULL, NULL, &dstRect.w, &dstRect.h);
	AddToRenderPool(renderImage_t{ result, nullptr, dstRect, MAX_LAYER }, dynamic);
}

//***************
// eRenderer::DrawDebugRectIso
// converts the given SDL_Rect into an isomectric box
// dynamic causes the result to move and scale with the camera,
// otherwise it draws to screen coordinates
// DEBUG: the input SDL_Rect must be in cartesian world-coordinates
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawDebugRectIso(const SDL_Color & color, const SDL_Rect & rect, bool dynamic) const {
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);
	std::vector<eVec2> fPoints(5);

	// DEBUG: pushed in counter-clockwise line-draw order
	// top-left, top-right, bottom-right, bottom-left, top-left (closes the rhombus)
	fPoints[0] = eVec2((float)rect.x, (float)rect.y);
	fPoints[1] = eVec2((float)(rect.x + rect.w), (float)rect.y);
	fPoints[2] = eVec2((float)(rect.x + rect.w), (float)(rect.y + rect.h));
	fPoints[3] = eVec2((float)rect.x, (float)(rect.y + rect.h));
	fPoints[4] = fPoints[0];

	// transfer to a format SDL_RenderDrawLines can use
	std::vector<SDL_Point> iPoints(5);
	for (int i = 0; i < fPoints.size(); i++) {
		eMath::CartesianToIsometric(fPoints[i].x, fPoints[i].y);
		fPoints[i] -= (game.GetCamera().GetAbsBounds() * dynamic);
		iPoints[i] = SDL_Point{ eMath::NearestInt(fPoints[i].x), eMath::NearestInt(fPoints[i].y) };
	}

	if (dynamic) {
		// set the render target, and scale according to camera zoom
		SDL_SetRenderTarget(internal_renderer, scalableTarget);
		SDL_RenderSetScale(internal_renderer, game.GetCamera().GetZoom(), game.GetCamera().GetZoom());

		// draw to the scalableTarget
		SDL_RenderDrawLines(internal_renderer, iPoints.data(), iPoints.size());

		// reset the render target to default, 
		// reset the renderer scale,
		// and transfer the scalableTarget
		SDL_SetRenderTarget(internal_renderer, NULL);
		SDL_RenderSetScale(internal_renderer, 1.0f, 1.0f);
		SDL_RenderCopy(internal_renderer, scalableTarget, NULL, NULL);
	} else {
		SDL_RenderDrawLines(internal_renderer, iPoints.data(), iPoints.size());
	}
	SDL_SetRenderDrawColor(internal_renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

//***************
// eRenderer::DrawDebugRect
// dynamic == true draws to the scalableTarget
// dynamic == false draws to the default render target
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawDebugRect(const SDL_Color & color, const SDL_Rect & rect, bool fill, bool dynamic) const {
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);

	// map the given cartesian rectangle to isometric map coordinates
	eVec2 topLeft = eVec2((float)rect.x, (float)rect.y);
	eMath::CartesianToIsometric(topLeft.x, topLeft.y);
	topLeft -= (game.GetCamera().GetAbsBounds() * dynamic);
	SDL_Rect isoMapRect = { eMath::NearestInt(topLeft.x), eMath::NearestInt(topLeft.y), rect.w, rect.h };

	if (dynamic) {
		// set the render target, and scale according to camera zoom
		SDL_SetRenderTarget(internal_renderer, scalableTarget);
		SDL_RenderSetScale(internal_renderer, game.GetCamera().GetZoom(), game.GetCamera().GetZoom());

		// draw to the scalableTarget
		fill ? SDL_RenderFillRect(internal_renderer, &isoMapRect)
			: SDL_RenderDrawRect(internal_renderer, &isoMapRect);

		// reset the render target to default, 
		// reset the renderer scale,
		// and transfer the scalableTarget
		SDL_SetRenderTarget(internal_renderer, NULL);
		SDL_RenderSetScale(internal_renderer, 1.0f, 1.0f);
		SDL_RenderCopy(internal_renderer, scalableTarget, NULL, NULL);
	}
	else {
		fill ? SDL_RenderFillRect(internal_renderer, &isoMapRect)
			: SDL_RenderDrawRect(internal_renderer, &isoMapRect);
	}
	SDL_SetRenderDrawColor(internal_renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

//***************
// eRenderer::DrawDebugRects
// dynamic == true draws to the scalableTarget
// dynamic == false draws to the default render target
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawDebugRects(const SDL_Color & color, const std::vector<SDL_Rect> & rects, bool fill, bool dynamic) const {
	SDL_SetRenderDrawColor(internal_renderer, color.r, color.g, color.b, color.a);

	// TODO: the origin of the rectangles will still have to be converted to isometric to appear in the correct map position
	// TODO: transter and scale the scalableTarget over the default render target at the end of this function (if dynamc == true)
	// TODO: modify the loop logic such that SDL_RenderFillRects or SDL_RenderDrawRects is used instead of invoking once per rect

	int(*drawFunc)(SDL_Renderer *, const SDL_Rect *);
	drawFunc = fill ? SDL_RenderFillRect : SDL_RenderDrawRect;

	SDL_SetRenderTarget(internal_renderer, dynamic ? scalableTarget : NULL);
	for (auto && iter : rects) {
		SDL_Rect screenRect{ eMath::NearestInt(iter.x - game.GetCamera().Origin().x),
							 eMath::NearestInt(iter.y - game.GetCamera().Origin().y),
							 iter.w,
							 iter.h };
		drawFunc(internal_renderer, &screenRect);
	}
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_SetRenderDrawColor(internal_renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

//***************
// eRenderer::DrawImage
// if frame is nullptr the entire image is drawn
// otherwise frame sets a sub-section of the image to draw
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(std::shared_ptr<eImage> image, const SDL_Rect * srcRect, const SDL_Rect * dstRect) const {
	SDL_RenderCopy(internal_renderer, image->Source(), srcRect, dstRect);
}

//***************
// eRenderer::OnScreen
//***************
bool eRenderer::OnScreen(const eVec2 & point) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y), 
									eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.ContainsPoint(point);
}

//***************
// eRenderer::OnScreen
//***************
bool eRenderer::OnScreen(const eBounds & bounds) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y),
								   eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.Overlaps(bounds);
}

//***************
// eRenderer::AddToRenderPool
// dynamic == false is used for overlays and/or HUD guis
// dynamic == true is used for scaling and translating groups of images together based on camera properties
// DEBUG: most-significant 2 bytes of priority were set using the layer during construction,
// the the least-significant 2 bytes are now set according to the order the renderImage was added to the renderPool
//***************
void eRenderer::AddToRenderPool(renderImage_t & renderImage, bool dynamic) {
	std::vector<renderImage_t> * targetPool = dynamic ? &dynamicPool : &staticPool;
	renderImage.priority |= targetPool->size();
	targetPool->push_back(renderImage);
}

//***************
// eRenderer::FlushDynamicPool
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
void eRenderer::FlushDynamicPool() {
	// sort the dynamicPool for the scalableTarget
	QuickSort(	dynamicPool.data(),
				dynamicPool.size(),
				[](auto && a, auto && b) {
				if (a.priority < b.priority) return -1;
				else if (a.priority > b.priority) return 1;
				return 0;
			});

	// set the render target, and scale according to camera zoom
	SDL_SetRenderTarget(internal_renderer, scalableTarget);
	SDL_RenderSetScale(internal_renderer, game.GetCamera().GetZoom(), game.GetCamera().GetZoom());

	// draw to the scalableTarget
	for (auto && iter : dynamicPool)
		DrawImage(iter.image, iter.srcRect, &iter.dstRect);

	dynamicPool.clear();

	// reset the render target to default, 
	// reset the renderer scale,
	// and transfer the scalableTarget
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_RenderSetScale(internal_renderer, 1.0f, 1.0f);
	SDL_RenderCopy(internal_renderer, scalableTarget, NULL, NULL);
}

//***************
// eRenderer::FlushStaticPool
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
void eRenderer::FlushStaticPool() {
	// sort the staticPool for the default render target
	QuickSort(staticPool.data(),
			  staticPool.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
			});

	for (auto && iter : staticPool)
		DrawImage(iter.image, iter.srcRect, &iter.dstRect);

	staticPool.clear();
}

