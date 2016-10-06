#include "Game.h"

Game::Game() {

}

// FIXME: this initialization error handling needs adjustment
bool Game::Init() {

	int error = 0;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		error++;

	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);

	if (!window)
		error++;

	backbuffer = SDL_GetWindowSurface(window);
	clear = SDL_LoadBMP("graphics/background.bmp");

	if (!backbuffer || !clear)
		error++;

	if (TTF_Init() == -1)
		error++;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		error++;

	if (!map.Init("graphics/tiles.bmp", this, 160, 120))
		error++;

	if (!entities.Init("graphics/hero.bmp", false, this))
		error++;

	if (error) {
		Shutdown(error);
		return false;
	}
	
	return true;

}

void Game::Shutdown( int error ) {

	char buffer[8];

	if (error) {
		sprintf_s(buffer, "%i", error);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", buffer, NULL);
	}

	FreeAssets();

	TTF_Quit();
	SDL_Quit();
}

void Game::FreeAssets() {

	entities.Free();
	map.Free();

	if (window)
		SDL_DestroyWindow(window);

	if (!font)
		TTF_CloseFont(font);
}

// Precondition: map and entities are non-null data
// TODO: check victory condition here or in the entities for alternate quit
bool Game::Run() {

	static SDL_Event	event;

	// FIXME: currently the clear image and the window are the same dimensions (issue in fullscreen/scaling?)
	// clear the window for fresh drawing
	SDL_BlitSurface(clear, NULL, backbuffer, NULL);

	map.Update();
	entities.Update();
	SDL_UpdateWindowSurface(window);

	SDL_Delay(20);	// TODO: make this dynamic based on how long each frame took to produce (smooth the framerate)

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				return false;
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == 3)
					map.BuildTiles();
				if (event.button.button == 1)
					entities.AddUserWaypoint(eVec2((float)(event.button.x + map.GetCamera()->x), 
												   (float)(event.button.y + map.GetCamera()->y)));
				break;
			}
		}
	}

	return true;
}

void Game::DrawOutlineText(char* string, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Surface* renderedText = NULL;
	SDL_Color color;
	SDL_Rect pos;

	color.r = r;
	color.g = g;
	color.b = b;

	renderedText = TTF_RenderText_Solid(font, string, color);

	pos.x = x;
	pos.y = y;

	SDL_BlitSurface(renderedText, NULL, backbuffer, &pos);
	SDL_FreeSurface(renderedText);
}

SDL_Surface* Game::GetBuffer() {

	return backbuffer;
}

Map* Game::GetMap() {
	
	return &map;
}

Entity* Game::GetEntities() {

	return &entities;
}
