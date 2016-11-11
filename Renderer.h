#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"

class eImage;
class eSprite;

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

	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;
	void				DrawImage(eImage * image, const eVec2 & point) const;
	void				DrawSprite(eSprite * sprite, const eVec2 & point) const;
	bool				FormatSurface(SDL_Surface ** surface, bool colorKey) const;
	void				DrawClearRect(const SDL_Rect & rect) const;

	bool				OnScreen(const eVec2 & point) const;
	bool				OnScreen(const eBounds & bounds) const;

private:

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

#endif /* EVIL_RENDERER_H */
