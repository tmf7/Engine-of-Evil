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

// BEGIN FREEHILL OBB calculation test
typedef struct OBB_s {
	eVec2 center;		// world-space center
	eVec2 axes[2];		// unit-length locally oriented x and y axes
	eVec2 halfSize;		// positive distance along local x and y axes
} OBB_t;

// DEBUG: edges must be perpendicular
// and corner must be their point of intersection
// TODO: now that a better representation is had
// ... do an obb-obb test?
// confirm I have the correct representation by drawing it?
// convert an AABB to an OBB representation to do the OBBOBB test
OBB_t GetOBB(eVec2 corner, eVec2 edges[2]) {
	OBB_t obb;

	eVec2 xAxis = -edges[1];
	xAxis.Normalize();
	eVec2 yAxis = -edges[0];
	yAxis.Normalize();
	obb.axes[0] = xAxis;
	obb.axes[1] = yAxis;

	obb.halfSize.y = edges[0].Length() / 2.0f;
	obb.halfSize.x = edges[1].Length() / 2.0f;

	eVec2 diagonal = edges[0] + edges[1];
	float centerDist = diagonal.Length() / 2.0f;
	diagonal.Normalize();
	obb.center = corner + (diagonal * centerDist);
	
	return obb;
}

// TODO: move this to eCollision
// test for a separating axis using 
// the 8 faces of both OBBs
bool OBBOBBTest(const OBB_t & a, const OBB_t & b) {
	float ra;
	float rb;
	float R[2][2];
	float AbsR[2][2];
	const float EPSILON = 0.015625f;	// 1/64 (because binary fractions)

	// DEBUG: all z-values of rotation matrix R are 0,
	// except z-z which would be R[2][2] if R were 3x3
	// so it is hereafter replaced with (1.0f + EPSILON)
	const float R22 = 1.0f + EPSILON;

	// compute rotation matrix spressing b in a' coordinate frame
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			R[i][j] = a.axes[i] * b.axes[j];

	// compute translation vector
	// and bring it into a's coordinate frame
	eVec2 t = b.center - a.center;
	t.Set(t * a.axes[0], t * a.axes[1]);

	// compute common subexpressions. add in an epsilon term to
	// counteract arithmetic erros when tow edges are parallel and
	// their corss product is (near) zero
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			AbsR[i][j] = abs(R[i][j]) + EPSILON;

	// test axes a.axes[0] and a.axes[1]
	for (int i = 0; i < 2; i++) {
		ra = a.halfSize[i];
		rb = b.halfSize[0] * AbsR[i][0] + b.halfSize[1] * AbsR[i][1];
		if (abs(t[i]) > ra + rb) return false;
	}

	// test axes b.axes[0] and b.axes[1]
	for (int i = 0; i < 2; i++) {
		ra = a.halfSize[0] * AbsR[0][i] + a.halfSize[1] * AbsR[1][i];
		rb = b.halfSize[i];
		if (abs(t[0] * R[0][i] + t[1] * R[1][i]) > ra + rb) return false;
	}
	
	// test axis a.axes[0] X b.axes[2] (which is [0,0,1] for 2D)
	ra = a.halfSize[1] * R22;
	rb = b.halfSize[0] * AbsR[0][1] + b.halfSize[1] * AbsR[0][0];
	if (abs(-t[1] * R22) > ra + rb) return false;

	// test axis a.axes[1] X b.axes[2] (which is [0,0,1] for 2D)
	ra = a.halfSize[0] * R22;
	rb = b.halfSize[0] * AbsR[1][1] + b.halfSize[1] * AbsR[1][0];
	if (abs(t[0] * R22) > ra + rb) return false;

	// test axis a.axes[2] (which is [0,0,1] for 2D) X b.axes[0]
	ra = a.halfSize[0] * AbsR[1][0] + a.halfSize[1] * AbsR[0][0];
	rb = b.halfSize[1] * R22;
	if (abs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;

	// test axis a.axes[2] (which is [0,0,1] for 2D) X b.axes[1]
	ra = a.halfSize[0] * AbsR[1][1] + a.halfSize[1] * AbsR[0][1];
	rb = b.halfSize[0] * R22;
	if (abs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;

	// no separating axis, OBBs intersecting
	return true;

}

// END FREEHILL OBB calculation test

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

// BEGIN FREEHILL DEBUG draw order checks

	// DEBUG: entity renderImage bounds (confirmed, translated into visually isometric map position)
	auto & renderImage = *entities[0]->GetRenderImage();
	auto renderOrigin = renderImage.origin;
	eBounds dstBounds = eBounds(renderOrigin, renderOrigin + eVec2((float)renderImage.srcRect->w, (float)renderImage.srcRect->h));
	renderer.DrawCartesianRect(greenColor, dstBounds, false, RENDERTYPE_DYNAMIC);

	// DEBUG: entity collision bounds (confirmed, 2D over the othographic logical map)
	renderer.DrawCartesianRect(redColor, entities[0]->CollisionModel().AbsBounds(), false, RENDERTYPE_DYNAMIC);

	// DEBUG: camera bounds location (confirmed, yes)
	renderer.DrawCartesianRect(greenColor, camera.CollisionModel().AbsBounds(), false, RENDERTYPE_DYNAMIC);

	// AABBAABBTest(camBounds, dstBounds) results: figured out why the entity disappears (VISUAL positions & hence overlap is bad)

	// DEBUG: isometric rect draw (dynam/static)
	renderer.DrawIsometricRect(redColor, entities[0]->CollisionModel().AbsBounds(), false, RENDERTYPE_DYNAMIC);

	// DEBUG: testing HOW the camera winds up drawing the correct gridcells
	float scale = 25.0f;
	eVec2 offset = eVec2(50.0f, 50.0f);
	// DEBUG: scaled 2D cartesian map area
	eBounds cartMapBounds = eBounds( offset, offset + (eVec2( map.TileMap().Width(), map.TileMap().Height()) / scale) );
	renderer.DrawCartesianRect(greenColor, cartMapBounds, false, RENDERTYPE_STATIC);

	// DEBUG: correct cells area on the isometric map that need to be drawn
	auto & camBounds = camera.CollisionModel().AbsBounds();
	auto topLeft = camBounds[0];
	auto topRight = eVec2(camBounds[1].x, camBounds[0].y);
	auto bottomLeft = eVec2(camBounds[0].x, camBounds[1].y);
	auto bottomRight = camBounds[1];

	eMath::IsometricToCartesian(topLeft.x, topLeft.y);
	eMath::IsometricToCartesian(topRight.x, topRight.y);
	eMath::IsometricToCartesian(bottomLeft.x, bottomLeft.y);
	eMath::IsometricToCartesian(bottomRight.x, bottomRight.y);
	topLeft = (topLeft / scale) + offset;
	topRight = (topRight / scale) + offset;
	bottomLeft = (bottomLeft / scale) + offset;
	bottomRight = (bottomRight / scale) + offset;

	// DEBUG: OBB conversion and collision function testing (THEY WORK!)
	eVec2 mapEdges[2] = {	eVec2(cartMapBounds[1].x, cartMapBounds[0].y) - cartMapBounds[0],
							eVec2(cartMapBounds[0].x, cartMapBounds[1].y) - cartMapBounds[0]	};
	OBB_t mapOBB = GetOBB(cartMapBounds[0], mapEdges);
	eVec2 camEdges[2] = { topRight - topLeft, bottomLeft - topLeft };
	OBB_t camOBB = GetOBB(topLeft, camEdges);
	if(OBBOBBTest(mapOBB, camOBB))
		renderer.DrawCartesianRect(blackColor, eBounds(vec2_zero, eVec2(32.0f, 32.0f)), true, RENDERTYPE_STATIC);

	std::array<SDL_Point, 5> iPoints;
	iPoints[0] = { eMath::NearestInt(topLeft.x), eMath::NearestInt(topLeft.y) };
	iPoints[1] = { eMath::NearestInt(topRight.x), eMath::NearestInt(topRight.y) };
	iPoints[2] = { eMath::NearestInt(bottomRight.x), eMath::NearestInt(bottomRight.y) };
	iPoints[3] = { eMath::NearestInt(bottomLeft.x), eMath::NearestInt(bottomLeft.y) };
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
