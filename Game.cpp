#include "Game.h"

//byte_t memoryPool[ONE_GIGABYTE];
eGame game;

//****************
// eGame::Init
//****************
eGame::ErrorCode eGame::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return SDL_ERROR;

	if (!renderer.Init())
		return RENDERER_ERROR;

	if (!imageManager.Init())
		return IMAGE_MANAGER_ERROR;

	if (!imageTilerManager.Init())
		return TILER_MANAGER_ERROR;

	if (!map.Init())
		return MAP_ERROR;

	input.Init();		// DEBUG: will crash if it fails (around the new allocation)
	camera.Init();

	entities.push_back(std::make_shared<eAI>());
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
			case IMAGE_MANAGER_ERROR:
				SDL_strlcpy(message, "IMAGE MANAGER INIT FAILURE", 64);
				break;
			case TILER_MANAGER_ERROR:
				SDL_strlcpy(message, "TILER MANAGER INIT FAILURE", 64);
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
// eGame::DrawFPS
// add fps text to the renderPool
//****************
void eGame::DrawFPS() {
	std::string fraps = "FPS: ";
	fraps += std::to_string(fixedFPS);
	fraps += "/";
	fraps += std::to_string(GetDynamicFPS());
	renderer.DrawOutlineText(fraps.c_str(), vec2_zero, redColor, false, RENDERTYPE_STATIC);
}

//****************
// eGame::Run
//****************
bool eGame::Run() {	
	Uint32 startTime = SDL_GetTicks();

	// FIXME/TODO: the event poll may be extremely slow
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return false;
	}
	
	// system updates
	input.Update();
	entities[0]->Think();			// TODO: loop over all entities
	camera.Think();
	map.Think();

	// draw the dynamic/scalable gameplay
	renderer.Clear();
	map.Draw();
	entities[0]->Draw();
	renderer.FlushDynamicPool();

	// TODO: write and call these DYNAMIC geometry debug draw calls
	// draw all debug information as an overlay
	//	map.DebugDraw();				// draw the collision bounds of collidable tiles
	//	entities[0].DebugDraw();		// loop over all entities for their collision bounds, and grid occupancy
										// ALSO: only draw goal/trail_waypoints and known_map for a SINGLE currently SELECTED entity

	// TODO: the HUD would be a static/non-scalable overlay...which should draw with the player...
	// BUT THAT'S FINE because FlushStaticPool gets called last, whew!

	// draw static debug information
	if (debugFlags.FRAMERATE)
		DrawFPS();

//	renderer.FlushStaticPool();			// DEBUG: not currently used
	renderer.Show();

	// frame-rate governing delay
	deltaTime = SDL_GetTicks() - startTime;

	// DEBUG: breakpoint handling
	if (deltaTime > 1000)
		deltaTime = frameTime;

	// DEBUG: delta time of this last frame is not used as the global update interval,
	// instead the globally available update interval is fixed to frameTime
	deltaTime <= frameTime ? SDL_Delay(frameTime - deltaTime) : SDL_Delay(deltaTime - frameTime);
	return true;
}
