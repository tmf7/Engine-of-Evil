#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "Definitions.h"
#include "Math.h"

class eImage;
class eSprite;
class eEntity;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the backbuffer and window handles.
//****************************************************
class eRenderer {
public:

						eRenderer();

	bool				Init();
	void				Free();
	void				Clear();
	void				Show();
	int					Width() const;
	int					Height() const;
	bool				OnScreen(const eVec2 & point) const;
	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b);
	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b);
	void				DrawImage(eImage * image, const eVec2 & point);
	void				DrawSprite(eSprite * sprite, const eVec2 & point);
	bool				FormatSurface(SDL_Surface ** surface, bool colorKey) const;

private:

	SDL_Surface *		backbuffer;
	SDL_Surface *		clear;
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
// FIXME: should just be color, to allow for resizable window/backbuffer and fullscreen
//***************
inline void eRenderer::Clear() {
	SDL_BlitSurface(clear, NULL, backbuffer, NULL);
}

//***************
// eRenderer::Show
//***************
inline void eRenderer::Show() {
	SDL_UpdateWindowSurface(window);
}

//***************
// eRenderer::OnScreen
// tests if a given x,y point is in the window area (backbuffer)
//***************
inline bool eRenderer::OnScreen(const eVec2 & point) const {

	if (point.x >= backbuffer->w || point.x < 0 ||
		point.y >= backbuffer->h || point.y < 0)
		return false;

	return true;
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

#endif /* EVIL_RENDERER_H */
