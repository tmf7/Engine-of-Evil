#include "Renderer.h"
#include "Image.h"
#include "Sprite.h"

//***************
// eRenderer::Init
// initialize the window, its backbuffer surface, and a default font
//***************
bool eRenderer::Init() {

	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if (!window)
		return false;

	internal_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!internal_renderer)
		return false;

	if (SDL_SetRenderDrawColor(internal_renderer, 0, 0, 0, 255) == -1)		// opaque black
		return false;

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

	if (!font)
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
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
	SDL_Surface * surfaceText = TTF_RenderText_Solid(font, string, SDL_Color{ r,g,b, a });
	if (surfaceText == NULL)
		return;

	SDL_Texture * renderedText = SDL_CreateTextureFromSurface(internal_renderer, surfaceText);
	if (renderedText == NULL)
		return;

	SDL_Rect destRect{ (int)point.x , (int)point.y };
	SDL_RenderCopy(internal_renderer, renderedText, NULL, &destRect);
	SDL_FreeSurface(surfaceText);
}

//***************
// eRenderer::DrawImage
// draws a source image's current frame to the screen
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(eImage * image, const eVec2 & point) const {
	SDL_Rect destRect;

	if (image == NULL || image->Source() == NULL)
		return;

	destRect.x = (int)point.x;
	destRect.y = (int)point.y;
	destRect.w = image->Frame().w;
	destRect.h = image->Frame().h;

	SDL_RenderCopy(internal_renderer, image->Source(), &image->Frame(), &destRect);
}
/*
//***************
// eRenderer::DrawPixel
// Draws a single pixel to the backbuffer
// after confirming the point is on the backbuffer
// DEBUG: converts the input point float data to integer values
// TODO: dont use this anymore
//***************
void eRenderer::DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const {
	Uint8 * targetPixel;
	Uint32 color;
	int x, y;

//	static Uint8 * backbufferStart = (Uint8 *)backbuffer->pixels;
//	static Uint8 * backbufferEnd = (Uint8 *)backbuffer->pixels + (backbuffer->h - 1) * backbuffer->pitch + (backbuffer->w - 1) * backbuffer->format->BytesPerPixel;

	if (!OnScreen(point))
		return;

	if (SDL_MUSTLOCK(backbuffer)) {
		if (SDL_LockSurface(backbuffer) < 0)
			return;
	}

	x = (int)point.x;
	y = (int)point.y;

	color = SDL_MapRGB(backbuffer->format, r, g, b);
	targetPixel = (Uint8 *)backbuffer->pixels + y * backbuffer->pitch + x * backbuffer->format->BytesPerPixel;
	*(Uint32 *)targetPixel = color;		// DEBUG: assumes dereferenced targetPixel is a Uint32

	if (SDL_MUSTLOCK(backbuffer))
		SDL_UnlockSurface(backbuffer);
}
*/
/*
//***************
// eRenderer::FormatSurface
// converts the given surface to the backbuffer format
// for color keying
// returns false on failure
//***************
bool eRenderer::FormatSurface(SDL_Surface ** surface, int * colorKey) const {
	SDL_Surface * formattedSurface;
	Uint32 colorMap;

	formattedSurface = SDL_ConvertSurface(*surface, backbuffer->format, SDL_RLEACCEL);
	if (formattedSurface == NULL)
		return false;
	
	SDL_FreeSurface(*surface);		// DEBUG: free the memory allocated for the original surface
	*surface = formattedSurface;		// point to the newly formatted surface memory block

	if (colorKey != nullptr) {
		colorMap = SDL_MapRGB((*surface)->format, colorKey[0], colorKey[1], colorKey[2]);
		SDL_SetColorKey(*surface, SDL_TRUE, colorMap);
	}
	return true;
}
*/

