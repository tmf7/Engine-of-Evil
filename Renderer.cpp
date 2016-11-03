#include "Renderer.h"
#include "Math.h"
#include "Image.h"
#include "Sprite.h"

//***************
// Renderer::Init
// initialize the window, its backbuffer surface, and a default font
//***************
bool Renderer::Init() {
	
	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);

	if (!window)
		return false;

	backbuffer = SDL_GetWindowSurface(window);
	clear = SDL_LoadBMP("graphics/clear.bmp");

	if (!backbuffer || !clear)
		return false;

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		return false;

	return true;
}

//***************
// Renderer::Free
// close the font and destroy the window
//***************
void Renderer::Free() {

	if (!font)
		TTF_CloseFont(font);

	TTF_Quit();

	if (window)
		SDL_DestroyWindow(window);
}

//***************
// Renderer::DrawOutlineText
// Draws a string to the backbuffer
//***************
void Renderer::DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Surface * renderedText = NULL;
	SDL_Color color;
	SDL_Rect destRect;

	if (!OnScreen(point))
		return;

	color.r = r;
	color.g = g;
	color.b = b;

	renderedText = TTF_RenderText_Solid(font, string, color);
	if (renderedText == NULL)
		return;

	destRect.x = (int)point.x;
	destRect.y = (int)point.y;

	SDL_BlitSurface(renderedText, NULL, backbuffer, &destRect);
	SDL_FreeSurface(renderedText);
}

//***************
// Renderer::DrawPixel
// Draws a single pixel to the backbuffer
//***************
void Renderer::DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) {
	Uint32 * buffer;
	Uint32 color;

	if (!OnScreen(point))
		return;

	if (SDL_MUSTLOCK(backbuffer)) {
		if (SDL_LockSurface(backbuffer) < 0)
			return;
	}

	color = SDL_MapRGB(backbuffer->format, r, g, b);

	buffer = (Uint32 *)backbuffer->pixels + (int)point.y * backbuffer->pitch / 4 + (int)point.x;
	(*buffer) = color;

	if (SDL_MUSTLOCK(backbuffer))
		SDL_UnlockSurface(backbuffer);
}

//***************
// Renderer::DrawImage
// draws a source image's current frame onto the backbuffer
//***************
void Renderer::DrawImage(Image * image, const eVec2 & point) {
	SDL_Rect destRect;

	if (image == NULL || image->Source() == NULL)
		return;

	if (!OnScreen(point))
		return;

	destRect.x = (int)point.x;
	destRect.y = (int)point.y;

	SDL_BlitSurface(image->Source(), image->Frame(), backbuffer, &destRect);
}

//***************
// Renderer::DrawSprite
// convenience function
// draws a sprites image's current frame onto the backbuffer
//***************
void Renderer::DrawSprite(Sprite * sprite, const eVec2 & point) {
	DrawImage(sprite->GetImage(), point);
}


