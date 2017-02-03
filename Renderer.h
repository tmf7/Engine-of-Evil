#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include <vector>

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"
#include "Image.h"

typedef enum {
	RENDERTYPE_STATIC,
	RENDERTYPE_DYNAMIC
} eRenderType;

typedef struct renderImage_s {
	std::shared_ptr<eImage>		image;			// for entities, this would be which frame of the eSprite is being drawn
	const SDL_Rect *			srcRect;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect					dstRect;		// where on the screen and what size
	Uint32						priority;

	renderImage_s()
		: image(nullptr),
		  srcRect(nullptr),
		  priority(MAX_LAYER << 16) {};
	
	renderImage_s(std::shared_ptr<eImage> image, const SDL_Rect * srcRect, const SDL_Rect & dstRect, const Uint8 layer)
		: image(image),
		  srcRect(srcRect), 
		  dstRect(dstRect),
		  priority(layer << 16) {};	// DEBUG: most-significant 2 bytes are layer
									// least-significant 2 bytes are renderPool push order
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
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

	void				AddToRenderPool(renderImage_t & renderImage, bool dynamic);
	void				Flush();

	void				DrawOutlineText(const char * text, const eVec2 & point, const SDL_Color & color, bool constText, bool dynamic) const;
	void				DrawImage(std::shared_ptr<eImage> image, const SDL_Rect * srcRect, const SDL_Rect * destRect) const;
	void				DrawDebugRect(const SDL_Color & color, const SDL_Rect & rect, bool fill, bool dynamic) const;
	void				DrawDebugRects(const SDL_Color & color, const std::vector<SDL_Rect> & rects, bool fill, bool dynamic) const;

	bool				OnScreen(const eVec2 & point) const;
	bool				OnScreen(const eBounds & bounds) const;

private:

	static const int				defaultRenderCapacity = 1024;// maximum separate items to indirectly draw from the pool
	std::vector<renderImage_t>		staticPool;					
	std::vector<renderImage_t>		dynamicPool;				

	SDL_Window *		window;
	SDL_Renderer *		internal_renderer;
	SDL_Texture *		scalableTarget;
	TTF_Font *			font;
};

// utility colors
extern const SDL_Color clearColor;
extern const SDL_Color black;

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
