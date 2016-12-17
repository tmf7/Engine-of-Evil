#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include <vector>

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"

class eImage;
class eSprite;

// TODO: build an array of these (based on their visiblity) 
// then write a eRenderer::Draw() function which Quicksorts them (or insertion sorts upon additions)
// and draws all renderEntities in order of z-depth using eRenderer::DrawImage(image, position)
typedef struct renderImage_s {
	eVec2			position;	// where on the screen
	eImage *		image;		// for entities, this would be which frame of the eSprite is being drawn
	int				zDepth;		// ultimately determines draw order

	renderImage_s()
		: position(vec2_zero), image(nullptr), zDepth(-999) {};
	renderImage_s(const eVec2 & position, eImage * image, const int zDepth)
		: position(position), image(image), zDepth(zDepth) {};
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the backbuffer, window, and font handles.
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

	void				AddToRenderQueue(const eVec2 & position, eImage * image, const int priority);
	void				FlushRenderQueue();

	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawImage(eImage * image, const eVec2 & point) const;
	bool				FormatSurface(SDL_Surface ** surface, bool colorKey) const;
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
//***************
// eRenderer::AddToRenderQueue
//***************
inline void eRenderer::AddToRenderQueue(const eVec2 & position, eImage * image, const int zDepth) {
	renderQueue.push_back(renderImage_t(position, image, zDepth));
}

//***************
// eRenderer::FlushRenderQueue
//***************
inline void eRenderer::FlushRenderQueue() {
	QuickSort(renderQueue.data(), 
			  renderQueue.size(), 
			  [](auto && a, auto && b) { 
					if (a.zDepth < b.zDepth) return -1; 
					else if (a.zDepth > b.zDepth) return 1; 
					return 0; 
				}
	);
	for (auto && it : renderQueue)
		DrawImage(it.image, it.position);

	renderQueue.clear();
}

#endif /* EVIL_RENDERER_H */
