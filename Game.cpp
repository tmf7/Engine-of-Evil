#include "Game.h"

//byte_t memoryPool[ONE_GIGABYTE];
Uint32 globalIDPool = 0;
eGame game;
eAI boss;

//****************
// eGame::Init
//****************
eGame::ErrorCode eGame::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return SDL_ERROR;

	if (!renderer.Init())
		return RENDERER_ERROR;

	if (!map.Init())
		return MAP_ERROR;

	input.Init();		// DEBUG: will crash if it fails (around the new allocation)
	camera.Init();

	numEntities++;
	entities[0] = &boss;
	if (!entities[0]->Spawn())
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
	renderer.Free();
}

//****************
// eGame::Run
// map and entities must already be initialized
// TODO: check victory condition here or in the entities for alternate quit
// TODO: modify the frame rate governance
//****************
bool eGame::Run() {
	static SDL_Event	event;					// FIXME: don't make this static
	Uint32				start, frameDuration;
	int					delay;
	static const int	fps = 60;				// FIXME: don't make this static

/*
// TODO: as an alternative method of timing in general
// BUG: subsequent calls to now() will affect calculated run times (ie later intervals may seem quicker)
#include <chrono>
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	printf("%I64i milliseconds\n\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());

*/
	
	start = SDL_GetTicks();

	// FIXME/TODO: the event poll may be extremely slow if it has to search through all events
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return false;
	}
	
	input.Update();
	entities[0]->Think();
	camera.Think();
	map.Think();

	renderer.Clear();

	// TODO: this draw order should be z-depth dependent on a per-entity/tile basis
	map.Draw();
	entities[0]->Draw();

	// DEBUG: new function that does all the final blitting
	renderer.Flush();

	renderer.Show();

	frameDuration = SDL_GetTicks() - start;	// DEBUG: always positive unless game runs for ~49 days
	delay = (1000 / fps) - frameDuration;
	SDL_Delay(delay > 0 ? delay : 0);		// FIXME: SDL_Delay isn't the most reliable frame delay method

	return true;
}
