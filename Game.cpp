#include "Game.h"

//byte_t memoryPool[ONE_GIGABYTE];
eGame game;
eAI boss;

//****************
// eGame::Init
//****************
eGame::ErrorCode eGame::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return SDL_ERROR;

	if (!renderer.Init())
		return RENDERER_ERROR;

	if (!map.Init())
		return MAP_ERROR;

	numEntities++;
	entities[0] = &boss;
	if (!(static_cast<eAI *>(entities[0])->Spawn()))
		return ENTITY_ERROR;		// FIXME: make this entityID dependent

// DEBUG: testing global static memory allocation, works (taskmanager shows 1GB in use for evil.exe; takes about 2 seconds to memset 1GB though, slow startup, ran quick after)
//	memset(memoryPool, 0xff, sizeof(byte_t) * ONE_GIGABYTE);

	return INIT_SUCCESS;
}

//****************
// eGame::Shutdown
//****************
void eGame::Shutdown(eGame::ErrorCode error) {
	char message[64];

	if (error != INIT_SUCCESS) {
		switch (error) {
			case SDL_ERROR:
				SDL_strlcpy(message, "SDL INIT FAILURE", 64);
				break;
			case RENDERER_ERROR:
				SDL_strlcpy(message, "RENDERER INIT FAILURE", 64);
				break;
			case MAP_ERROR:
				SDL_strlcpy(message, "MAP INIT FAILURE", 64);
				break;
			case ENTITY_ERROR:
				SDL_strlcpy(message, "ENTITY (entityID_or_name_here) INIT FAILURE", 64);
				break;
		}
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message, NULL);
	}

	FreeAssets();
	SDL_Quit();
}

//****************
// eGame::FreeAssets
//****************
void eGame::FreeAssets() {
	imageManager.Free();
	renderer.Free();
}

//****************
// eGame::Run
// map and entities must already be initialized
// TODO: check victory condition here or in the entities for alternate quit
//****************
bool eGame::Run() {
	static SDL_Event	event;
	Uint32				start, frameDuration;
	int					delay;
	static const int	fps = 30;
	
	start = SDL_GetTicks();
	renderer.Clear();

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return false;
			}
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == 3)
					map.ToggleTile(eVec2((float)(event.button.x + camera.GetAbsBounds().x),
										 (float)(event.button.y + camera.GetAbsBounds().y)));
				if (event.button.button == 1)
					static_cast<eAI *>(entities[0])->AddUserWaypoint(eVec2((float)(event.button.x + camera.GetAbsBounds().x),
												   (float)(event.button.y + camera.GetAbsBounds().y)));
				break;
			}
			case SDL_KEYDOWN: {
				if (event.key.keysym.scancode == SDL_SCANCODE_0)
					map.BuildTiles(TRAVERSABLE_TILE);					// set entire map to brick
				else if (event.key.keysym.scancode == SDL_SCANCODE_1)
					map.BuildTiles(COLLISION_TILE);					// set entire map to water
				else if (event.key.keysym.scancode == SDL_SCANCODE_2)
					map.BuildTiles(RANDOM_TILE);						// set entire map random (the old way)
				break;
			}
		}
	}

	static_cast<eAI *>(entities[0])->Think();		// FIXME/BUG: this should call eAI::Think()
	camera.Think();
	map.Draw();
	static_cast<eAI *>(entities[0])->Draw();		// FIXME/BUG: this should call eAI::Think()
	renderer.Show();

	frameDuration = SDL_GetTicks() - start;	// NOTE: always positive unless game runs for ~49 days
	delay = (1000 / fps) - frameDuration;
	SDL_Delay(delay > 0 ? delay : 0);

	return true;
}
