#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "Definitions.h"

class eVec2;
class Image;
class Sprite;
class Entity;

//**************************************************
//				Renderer
// Base class for all window/fullscreen drawing. 
// Contains the backbuffer and window handles.
//****************************************************
class Renderer {
public:

						Renderer();
	bool				Init();
	void				Free();
	void				Clear();
	void				Show();
	bool				OnScreen(const eVec2 & point) const;
	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b);
	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b);
	void				DrawImage(Image * image, const eVec2 & point);
	void				DrawSprite(Sprite * sprite, const eVec2 & point);

private:

	SDL_Surface *		backbuffer;
	SDL_Surface *		clear;
	SDL_Window *		window;
	TTF_Font *			font;
};

//***************
// Renderer::Renderer
//***************
inline Renderer::Renderer() {
}

//***************
// Renderer::Clear
// resets the backbuffer to the clear image
// FIXME: should just be color, to allow for resizable window/backbuffer and fullscreen
//***************
inline void Renderer::Clear() {
	SDL_BlitSurface(clear, NULL, backbuffer, NULL);
}

//***************
// Renderer::Show
//***************
inline void Renderer::Show() {
	SDL_UpdateWindowSurface(window);
}

//***************
// Renderer::OnScreen
// tests if a given x,y point is in the window area (backbuffer)
//***************
inline bool Renderer::OnScreen(const eVec2 & point) const {

	if (point.x >= backbuffer->w || point.x < 0 ||
		point.y >= backbuffer->h || point.y < 0)
		return false;

	return true;
}

#endif /* EVIL_RENDERER_H */
