#include "Renderer.h"
#include "Image.h"
#include "Sprite.h"

//***************
// eRenderer::Init
// initialize the window, its backbuffer surface, and a default font
//***************
bool eRenderer::Init() {

	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);

	if (!window)
		return false;

	backbuffer = SDL_GetWindowSurface(window);

	if (!backbuffer)
		return false;

	clearColor = SDL_MapRGB(backbuffer->format, 0, 0, 0);	// black

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		return false;

	screenBounds[1] = eVec2((float)backbuffer->w - 1.0f, (float)backbuffer->h - 1.0f);	// TODO:  allow the window to be resized and the screenBounds with it

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

	if (window)
		SDL_DestroyWindow(window);
}

//***************
// eRenderer::DrawOutlineText
// Draws a string to the backbuffer
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const {
	SDL_Surface * renderedText;
	SDL_Color color;
	SDL_Rect destRect;

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
// eRenderer::DrawPixel
// Draws a single pixel to the backbuffer
// after confirming the point is on the backbuffer
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const {
	Uint8 * targetPixel;
	Uint32 color;
	int x, y;

//	static Uint8 * backbufferStart = (Uint8 *)backbuffer->pixels;
//	static Uint8 * backbufferEnd = (Uint8 *)backbuffer->pixels + (backbuffer->h - 1) * backbuffer->pitch + (backbuffer->w - 1) * backbuffer->format->BytesPerPixel;

	if (!screenBounds.ContainsPoint(point))
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

//***************
// eRenderer::DrawImage
// draws a source image's current frame onto the backbuffer
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(eImage * image, const eVec2 & point) const {
	SDL_Rect destRect;

	if (image == NULL || image->Source() == NULL)
		return;

	destRect.x = (int)point.x;
	destRect.y = (int)point.y;

	SDL_BlitSurface(image->Source(), &image->Frame(), backbuffer, &destRect);
}

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


