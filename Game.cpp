#include "Game.h"

Game::ErrorCode Game::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return SDL_ERROR;

	if (!renderer.Init())
		return RENDERER_ERROR;

	if (!map.Init("graphics/tiles.bmp", this, 160, 120))
		return MAP_ERROR;

	numEntities++;
	if (!entities[0].Init("graphics/hero.bmp", false, this))
		return ENTITY_ERROR;		// FIXME: make this entityID dependent

	return INIT_SUCCESS;
}

void Game::Shutdown(Game::ErrorCode error) {

	char message[8];

	if (error != INIT_SUCCESS) {
		switch (error) {
			case SDL_ERROR:
				strcpy(message, "SDL INIT FAILURE");
				break;
			case RENDERER_ERROR:
				strcpy(message, "RENDERER INIT FAILURE");
				break;
			case MAP_ERROR:
				strcpy(message, "MAP INIT FAILURE");
				break;
			case ENTITY_ERROR:
				strcpy(message, "ENTITY (entityID_or_name_here) INIT FAILURE");
				break;
		}
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message, NULL);
	}

	FreeAssets();
	SDL_Quit();
}

void Game::FreeAssets() {
	entities[0].Free();		// TODO: make this a pointer loop
	map.Free();
	renderer.Free();
}

// Precondition: map and entities are non-null data
// TODO: check victory condition here or in the entities for alternate quit
bool Game::Run() {
	static SDL_Event	event;
	Uint32 start, frameDuration, delay;
	static const int fps = 60;
	
	start = SDL_GetTicks();
	renderer.Clear();

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return false;
			}
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == 3)
					map.ToggleTile(eVec2((float)(event.button.x + map.camera.position.x),
										 (float)(event.button.y + map.camera.position.y)));
				if (event.button.button == 1)
					entities[0].AddUserWaypoint(eVec2((float)(event.button.x + map.camera.position.x), 
												   (float)(event.button.y + map.camera.position.y)));
				break;
			}
			case SDL_KEYDOWN: {
				if (event.key.keysym.scancode == SDL_SCANCODE_0)
					map.BuildTiles(Map::TRAVERSABLE_TILE);					// set entire map to brick
				else if (event.key.keysym.scancode == SDL_SCANCODE_1)
					map.BuildTiles(Map::COLLISION_TILE);					// set entire map to water
				else if (event.key.keysym.scancode == SDL_SCANCODE_2)
					map.BuildTiles(Map::RANDOM_TILE);						// set entire map random (the old way)
				break;
			}
		}
	}

	map.Update();
	entities[0].Update();
	renderer.Show();

	frameDuration = SDL_GetTicks() - start;	// NOTE: always positive unless game runs for ~49 days
	delay = (1000 / fps) - frameDuration;
	SDL_Delay(delay > 0 ? delay : 0);

	return true;
}
