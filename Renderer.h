#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include <vector>

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"

class eImage;
class eSprite;

typedef struct renderImage_s {
	eVec2			position;	// where on the screen
	eImage *		image;		// for entities, this would be which frame of the eSprite is being drawn
	size_t			priority;

	renderImage_s()
		: position(vec2_zero), image(nullptr), priority(UINT_MAX) {};

	renderImage_s(const eVec2 & position, eImage * image, const int layer)
		: position(position), image(image), priority(layer << 16) {};
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the backbuffer, window, and font handles.
// TODO: add renderer context handle (SDL_Renderer) for proper openGL code
//****************************************************
class eRenderer {
public:

						eRenderer();

	bool				Init();
	void				Free() const;
	void				Clear() const;
	void				Show() const;
	int					Width() const;
	int					Height() const;

	void				AddToRenderQueue(renderImage_t & renderImage);
	void				FlushRenderQueue();

	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawImage(eImage * image, const eVec2 & point) const;
	bool				FormatSurface(SDL_Surface ** surface, int * colorKey) const;
	void				DrawClearRect(const SDL_Rect & rect) const;

	bool				OnScreen(const eVec2 & point) const;
	bool				OnScreen(const eBounds & bounds) const;

private:

	static const int				defaultRenderCapacity = 1024;
	std::vector<renderImage_t>		renderQueue;

	eBounds				screenBounds;
	Uint32				clearColor;
	SDL_Surface *		backbuffer;
	SDL_Window *		window;
//	SDL_Renderer *		renderer;
	TTF_Font *			font;
};

//***************
// eRenderer::eRenderer
//***************
inline eRenderer::eRenderer() {
	renderQueue.reserve(defaultRenderCapacity);
}

//***************
// eRenderer::Clear
// resets the backbuffer to the clear image
//***************
inline void eRenderer::Clear() const {
	SDL_FillRect(backbuffer, NULL, clearColor);
//	SDL_BlitSurface(clear, NULL, backbuffer, NULL);
}

//***************
// eRenderer::Show
// updates the visible screen area
//***************
inline void eRenderer::Show() const {
	SDL_UpdateWindowSurface(window);
}

//***************
// eRenderer::Width
//***************
inline int eRenderer::Width() const {
	return backbuffer->w;
}

//***************
// eRenderer::Height
//***************
inline int eRenderer::Height() const {
	return backbuffer->h;
}

//***************
// eRenderer::OnScreen
//***************
inline bool eRenderer::OnScreen(const eVec2 & point) const {
	return screenBounds.ContainsPoint(point);
}

//***************
// eRenderer::OnScreen
//***************
inline bool eRenderer::OnScreen(const eBounds & bounds) const {
	return screenBounds.Overlaps(bounds);
}

//***************
// eRenderer::DrawClearRect
// sets the specified area of the screen to the clearColor
//***************
inline void eRenderer::DrawClearRect(const SDL_Rect & rect) const {
	SDL_FillRect(backbuffer, &rect, clearColor);
}

////////////////////////////////////////////////////////////////////////////////
// BEGIN FREEHILL draw order sort test
//***************
// eRenderer::AddToRenderQueue
// DEBUG: the second 2 bytes of priority were set using the layer during construction,
// the the first 2 bytes are now set according to the order the renderImage was added to the renderQueue
//***************
inline void eRenderer::AddToRenderQueue(renderImage_t & renderImage) {
	renderImage.priority |= renderQueue.size();
	renderQueue.push_back(renderImage);
}

//***************
// eRenderer::FlushRenderQueue
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
inline void eRenderer::FlushRenderQueue() {
	QuickSort(renderQueue.data(), 
			  renderQueue.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
				}
	);
	for (auto && iter : renderQueue)
		DrawImage(iter.image, iter.position);

	renderQueue.clear();
}
// END FREEHILL draw order sort test
#endif /* EVIL_RENDERER_H */
