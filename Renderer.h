#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"
#include "Image.h"

// eRenderType_t
typedef enum {
	RENDERTYPE_STATIC,
	RENDERTYPE_DYNAMIC
} eRenderType_t;

// renderImage_t
// TODO: only pass an eEntity's renderImage_t if it's bounds overlap the camera bounds
// THEN adjust to camera position
typedef struct renderImage_s {
	std::shared_ptr<eImage>		image;			// source image (ie texture wrapper)
	const SDL_Rect *			srcRect;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect					dstRect;		// SDL consumable cliprect, where on the screen (adjusted for camera position)
	eVec2						origin;			// top-left corner of image using world coordinates (not adjusted to account for camera)
												// DEBUG: dimensions relative to srcRect will affect scaling
	Uint32						priority;		// lower priority draws first
//	eEntity *					owner;			// who's using this renderimage, for secondary renderPool sort if priority causes flicker

	renderImage_s()
		: image(nullptr),
		  srcRect(nullptr),
		  priority(MAX_LAYER << 16) {};
	
	renderImage_s(std::shared_ptr<eImage> & image, const SDL_Rect * srcRect, const eVec2 & origin, const Uint8 layer)
		: image(image),
		  srcRect(srcRect), 
		  origin(origin),
		  priority(layer << 16) {};	// DEBUG: most-significant 2 bytes are layer
									// least-significant 2 bytes are renderPool push order

	void SetLayer(const int layer) {
		priority = layer << 16;
	}

	Uint32 GetLayer() const {
		return priority >> 16;
	}
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
// DynamicPool draws to a scalable target that typically tracks with the camera
// StaticPool draws so a non-scalable target that typically draws to screen coordinates
// DEBUG: RENDERTYPE_STATIC items must always be drawn last 
// otherwise they will be overwritten by the scalable target
// and its RENDERTYPE_DYANMIC items
//****************************************************
class eRenderer {
public:

						eRenderer();
						
	bool				Init();
	void				Free() const;
	void				Clear() const;
	void				Show() const;
	SDL_Rect			ViewArea() const;

	SDL_Renderer *		GetSDLRenderer() const;
	SDL_Window *		GetWindow() const;

	void				AddToRenderPool(renderImage_t * renderImage, bool dynamic);
	void				FlushDynamicPool();
	void				FlushStaticPool();

	void				DrawOutlineText(const char * text, eVec2 & point, const SDL_Color & color, bool constText, bool dynamic);
	void				DrawImage(const renderImage_t * renderImage) const;
	void				DrawDebugRectIso(const SDL_Color & color, const SDL_Rect & rect, bool dynamic) const;
	void				DrawDebugRect(const SDL_Color & color, const SDL_Rect & rect, bool fill, bool dynamic) const;

private:

	static const int					defaultRenderCapacity = 1024;// maximum number of items to draw using each render pool
	std::vector<renderImage_t *>		staticPool;					
	std::vector<renderImage_t *>		dynamicPool;				

	SDL_Window *		window;
	SDL_Renderer *		internal_renderer;
	SDL_Texture *		scalableTarget;
	TTF_Font *			font;
};

// utility colors
extern const SDL_Color clearColor;
extern const SDL_Color blackColor;
extern const SDL_Color greyColor_trans;
extern const SDL_Color greenColor;
extern const SDL_Color redColor;

//***************
// eRenderer::eRenderer
//***************
inline eRenderer::eRenderer() {
	staticPool.reserve(defaultRenderCapacity);
	dynamicPool.reserve(defaultRenderCapacity);
}

//***************
// eRenderer::Clear
//***************
inline void eRenderer::Clear() const {
	SDL_SetRenderTarget(internal_renderer, scalableTarget);
	SDL_RenderClear(internal_renderer);
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_RenderClear(internal_renderer);
}

//***************
// eRenderer::Show
// updates the visible screen area
//***************
inline void eRenderer::Show() const {
	SDL_RenderPresent(internal_renderer);
}

//***************
// eRenderer::ViewArea
//***************
inline SDL_Rect eRenderer::ViewArea() const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	return viewArea;
}

//***************
// eRenderer::GetRenderer
//***************
inline SDL_Renderer * eRenderer::GetSDLRenderer() const {
	return internal_renderer;
}

//*****************
// eRenderer::GetWindow
//*****************
inline SDL_Window * eRenderer::GetWindow() const {
	return window;
}

#endif /* EVIL_RENDERER_H */
