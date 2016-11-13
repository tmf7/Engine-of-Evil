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

	if (!input.Init())
		return INPUT_ERROR;

	camera.Init();

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
			case INPUT_ERROR:
				SDL_strlcpy(message, "INPUT INIT FAILURE", 64);
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

	// FIXME/TODO: the event poll may be extremely slow if it has to search through all events
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				return false;
			}
		}
	}
	
	input.Update();
	static_cast<eAI *>(entities[0])->Think();		// FIXME/BUG: this should call eAI::Think() (check dooms game::RunFrame)
	camera.Think();
	map.Think();

	renderer.Clear();
	map.Draw();
	static_cast<eAI *>(entities[0])->Draw();		// FIXME/BUG: this should call eAI::Draw()
	renderer.Show();

	frameDuration = SDL_GetTicks() - start;	// NOTE: always positive unless game runs for ~49 days
	delay = (1000 / fps) - frameDuration;
	SDL_Delay(delay > 0 ? delay : 0);

	return true;
}
