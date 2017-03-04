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

//	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);	//  SDL_LOG_PRIORITY_CRITICAL // 
//	SDL_LogSetOutputFunction(FileLogFn_ptr, NULL);	// DEGUG: SDL_LogCritical is called alot for some reason, bottleneck (DrawOutlineText, IMG_Load seem to be the reason)

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
	camera.Think();
	entities[0]->Think();			// TODO: loop over all entities
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

// BEGIN FREEHILL DEBUG draw order checks

	// DEBUG: testing that the entity renderImage bounds is where I think it is (no?)
	auto & renderImage = *entities[0]->GetRenderImage();
	eBounds dstBounds = eBounds(renderImage.origin, renderImage.origin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h));
	SDL_Rect rendRect = { (int)dstBounds[0].x, (int)dstBounds[0].y, (int)dstBounds.Width(), (int)dstBounds.Height() };
//	renderer.DrawCartesianRect(greenColor, rendRect, false, RENDERTYPE_DYNAMIC, false);

	// DEBUG: testing that the entity collision bounds is where I think it is (no?)
	auto & collBounds = entities[0]->CollisionModel().AbsBounds();
	SDL_Rect collRect = { (int)collBounds[0].x, (int)collBounds[0].y, (int)collBounds.Width(), (int)collBounds.Height() };
//	renderer.DrawCartesianRect(redColor, collRect, false, RENDERTYPE_DYNAMIC, false);

	// DEBUG: testing that the camera is where it think it is (confirmed, yes)
	auto & camBounds = camera.CollisionModel().AbsBounds();
	SDL_Rect camRect = { (int)camBounds[0].x, (int)camBounds[0].y, (int)camBounds.Width(), (int)camBounds.Height() };
//	renderer.DrawCartesianRect(greenColor, camRect, false, RENDERTYPE_DYNAMIC, false);

	// AABBAABBTest(camBounds, dstBounds) results: figured out why the entity disappears (positions & hence overlap is bad)

	// DEBUG: testing HOW the camera winds up drawing the correct gridcells
	int scale = 25;
	// DEBUG: scaled 2D cartesian map area
	SDL_Rect cartMapRect = { 50, 50, map.TileMap().Width() / scale, map.TileMap().Height() / scale };
	renderer.DrawCartesianRect(greenColor, cartMapRect, false, RENDERTYPE_STATIC, false);

	// DEBUG: correct cells on the isometric map that need to be drawn
	auto topLeft = camBounds[0];
	auto topRight = eVec2(camBounds[1].x, camBounds[0].y);
	auto bottomLeft = eVec2(camBounds[0].x, camBounds[1].y);
	auto bottomRight = camBounds[1];

	eMath::IsometricToCartesian(topLeft.x, topLeft.y);
	eMath::IsometricToCartesian(topRight.x, topRight.y);
	eMath::IsometricToCartesian(bottomLeft.x, bottomLeft.y);
	eMath::IsometricToCartesian(bottomRight.x, bottomRight.y);

	std::array<SDL_Point, 5> iPoints;
	iPoints[0] = { eMath::NearestInt(topLeft.x) / scale + 50, eMath::NearestInt(topLeft.y) / scale + 50 };
	iPoints[1] = { eMath::NearestInt(topRight.x) / scale + 50, eMath::NearestInt(topRight.y) / scale + 50 };
	iPoints[2] = { eMath::NearestInt(bottomRight.x) / scale + 50, eMath::NearestInt(bottomRight.y) / scale + 50 };
	iPoints[3] = { eMath::NearestInt(bottomLeft.x) / scale + 50, eMath::NearestInt(bottomLeft.y) / scale + 50 };
	iPoints[4] = iPoints[0];
	SDL_SetRenderDrawColor(renderer.GetSDLRenderer(), redColor.r, redColor.g, redColor.b, redColor.a);
	SDL_RenderDrawLines(renderer.GetSDLRenderer(), iPoints.data(), iPoints.size());
	SDL_SetRenderDrawColor(renderer.GetSDLRenderer(), clearColor.r, clearColor.g, clearColor.b, clearColor.a);

// END FREEHILL DEBUG draw order checks

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
