#include "Game.h"

//byte_t memoryPool[ONE_GIGABYTE];
eGame game;

/*
//*************
// (global) FileLogFn
// output all SDL_LogXYZ messages to a log.txt file
//************
void FileLogFn(void * userdata, int category, SDL_LogPriority priority, const char * message) {
	std::ofstream write("EvilErrorLog.txt");

	// unable to find/create/open file
	if (!write.good()) 
		return;

	write.seekp(0, std::ios_base::end);
	write << message << '\n';

	write.close();
}
SDL_LogOutputFunction FileLogFn_ptr = &FileLogFn;
*/

//****************
// eGame::Init
//****************
eGame::ErrorCode eGame::Init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return SDL_ERROR;

//	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);	// SDL_LOG_PRIORITY_CRITICAL // 
//	SDL_LogSetOutputFunction(FileLogFn_ptr, NULL);		// DEBUG: SDL_LogCritical is called alot for some reason, bottleneck (DrawOutlineText, IMG_Load seem to be the reason)

	if (!renderer.Init())
		return RENDERER_ERROR;

	if (!imageManager.Init())
		return IMAGE_MANAGER_ERROR;

	if (!animationManager.Init())
		return ANIMATION_MANAGER_ERROR;

	if (!animationControllerManager.Init())
		return ANIMATION_CONTROLLER_MANAGER_ERROR;

//	try {
		input.Init();
//	} catch (const std::bad_alloc & e) {
//		throw;
//		return INPUT_ERROR;						// TODO: log a proper error
//	}

	camera.Init();

	if (!entityPrefabManager.Init())
		return ENTITY_PREFAB_MANAGER_ERROR;

	if (!map.Init())
		return MAP_ERROR;


// DEBUG: testing global static memory allocation, works (taskmanager shows 1GB in use for evil.exe; takes about 2 seconds to memset 1GB though, slow startup, ran quick after)
//	memset(memoryPool, 0xff, sizeof(byte_t) * ONE_GIGABYTE);
	
	gameTime = SDL_GetTicks();

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
			case ANIMATION_MANAGER_ERROR:
				SDL_strlcpy(message, "ANIMATION MANAGER INIT FAILURE", 64);
				break;
			case ANIMATION_CONTROLLER_MANAGER_ERROR:
				SDL_strlcpy(message, "ANIMATION CONTROLER MANAGER INIT FAILURE", 64);
				break;
			case ENTITY_PREFAB_MANAGER_ERROR:
				SDL_strlcpy(message, "ENTITY PREFAB MANAGER INIT FAILURE", 64);
				break;
			case MAP_ERROR:
				SDL_strlcpy(message, "MAP INIT FAILURE", 64);
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
// eGame::AddEntity
// finds the first unused slot in game::entities to move param entity
// and assigns it a spawnID
// returns the new spawnID index within game::entities
// returns -1 if something went wrong
//****************
int eGame::AddEntity(std::unique_ptr<eEntity> && entity) {
	int spawnID = 0;
	for (auto & entitySlot : entities) {
		if (entitySlot == nullptr) {
			entitySlot = std::move(entity);
			return spawnID;
		} else {
			++spawnID;
		}
	}

	if (spawnID == entities.size()) {
		entities.emplace_back(std::move(entity));
		return spawnID;
	}
	return -1;
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

	player.Think();

	for (auto & entity : entities) {
		entity->UpdateComponents();
		entity->Think();
	}

	camera.Think();

	// draw the dynamic/scalable gameplay
	renderer.Clear();
	map.Draw();
	renderer.FlushCameraPool();
	player.Draw();

	// all debug information is an overlay
	for (auto & entity : entities)
		entity->DebugDraw();		

	// TODO(!): modify eRenderer shape and text drawing to be on secondary textures to be referenced via renderImages
	// and added to the static/dynamic-renderPools for more flexible debug (or otherwise) drawing (ie just one ::Draw call here)
	map.DebugDraw();				// draw the collision bounds of collidable tiles
	player.DebugDraw();

	// TODO: the HUD would be a static/non-scalable overlay...which should draw with the player...
	// BUT THAT'S FINE because FlushStaticPool gets called last, whew!

	// draw static debug information
	if (debugFlags.FRAMERATE)
		DrawFPS();

//	renderer.FlushOverlayPool();			// DEBUG: not currently used
	renderer.Show();

	// frame-rate governing delay
	gameTime = SDL_GetTicks();
	deltaTime = gameTime - startTime;

	// DEBUG: breakpoint handling
	if (deltaTime > 1000)
		deltaTime = frameTime;

	// DEBUG: delta time of this last frame is not used as the global update interval,
	// instead the globally available update interval is fixed to frameTime
	deltaTime <= frameTime ? SDL_Delay(frameTime - deltaTime) : SDL_Delay(deltaTime - frameTime);
	return true;
}
