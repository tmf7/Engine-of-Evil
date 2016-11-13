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
//	clear = SDL_LoadBMP("graphics/clear.bmp");

	if (!backbuffer)// || !clear
		return false;

	clearColor = SDL_MapRGB(backbuffer->format, 0, 0, 0);	// black

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		return false;

	screenBounds[1] = eVec2((float)backbuffer->w, (float)backbuffer->h);		// TODO:  allow the window to be resized and the screenBounds with it

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
// user can optimize by checking eRenderer::OnScreen(point) == true;
// NOTE: converts the input point float data to integer values
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
//***************
void eRenderer::DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const {
	Uint32 * buffer;
	Uint32 color;

	if (!screenBounds.ContainsPoint(point))
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
// eRenderer::DrawImage
// draws a source image's current frame onto the backbuffer
// user can optimize by checking eRenderer::OnScreen(point) == true;
// NOTE: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(eImage * image, const eVec2 & point) const {
	SDL_Rect destRect;

	if (image == NULL || image->Source() == NULL)
		return;

	destRect.x = (int)point.x;
	destRect.y = (int)point.y;

	SDL_BlitSurface(image->Source(), image->Frame(), backbuffer, &destRect);
}

//***************
// eRenderer::DrawSprite
// convenience function
// draws a sprites image's current frame onto the backbuffer
// user can optimize by checking eRenderer::OnScreen(point) == true;
//***************
void eRenderer::DrawSprite(eSprite * sprite, const eVec2 & point) const {
	DrawImage(sprite->Image(), point);
}

//***************
// eRenderer::FormatSurface
// converts the given surface to the backbuffer format
// returns false on failure
// TODO: make the colorKey paramater more flexible (eg: user-defined keys and/or alpha values)
//***************
bool eRenderer::FormatSurface(SDL_Surface ** surface, bool colorKey) const {
	SDL_Surface * formattedSurface;
	Uint32 colorMap;
	int pink[3] = {255, 0, 255};

	formattedSurface = SDL_ConvertSurface(*surface, backbuffer->format, 0);
	if (formattedSurface == NULL)
		return false;

	SDL_FreeSurface(*surface);		// free the memory allocated for the original surface
	*surface = formattedSurface;		// point to the newly formatted surface memory block
	
	if (colorKey) {
		colorMap = SDL_MapRGB((*surface)->format, pink[0], pink[1], pink[2]);
		SDL_SetColorKey(*surface, SDL_TRUE, colorMap);
	}
	return true;
}


