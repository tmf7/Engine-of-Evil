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

// DEBUG: additional functionality for eBounds to facilitate
// transforming to OBB (eBox)
// DEBUG: points is filled in a clockwise winding order from topleft 
void ToPoints(const eBounds & bounds, eVec2 points[4]) {
	for (int i = 0; i < 4; i++) {
		points[i][0] = bounds[(i^(i>>1))&1][0];
		points[i][1] = bounds[(i>>1)&1][1];
	}
}

// DEBUG: testing arbitrary rotation about an arbitrary origin
void RotatePoint(eVec2 & point, const float degrees, const eVec2 & origin) {
	const float cosAngle = SDL_cosf(DEG2RAD(degrees));
	const float sinAngle = SDL_sinf(DEG2RAD(degrees));
	const eVec2 R[2] = { eVec2(cosAngle, sinAngle),
						 eVec2(-sinAngle, cosAngle) };

	// move to the world origin
	eVec2 localPoint = point - origin;
	point = origin + eVec2(	R[0].x * localPoint.x + R[1].x * localPoint.y,
							R[0].y * localPoint.x + R[1].y * localPoint.y	);
}


// produce a fitted AABB from a list of random points 
// DEBUG: in this test case the four corners of an OBB
// but this method works with any set of points any size
eBounds GetAABBFromPoints(std::array<eVec2, 4> & points) {
	static const std::array<eVec2, 2> axes = { eVec2(1.0f, 0.0f), eVec2(0.0f, 1.0f) };
	eVec2 mins;
	eVec2 maxs;

	// find the max and min projection coordinate along each axis
	for (int i = 0; i < 2; i++) {
		float minproj = FLT_MAX;
		float maxproj = -FLT_MAX;
		for (int j = 0; j < points.size(); j++) {
			float proj = points[j] * axes[i];
			if (proj < minproj) {
				minproj = proj;
				mins[i] = points[j][i];
			}
			if (proj > maxproj) {
				maxproj = proj;
				maxs[i] = points[j][i];
			}
		}
	}
	return eBounds(mins, maxs);
}

// Transform AABB a by the matrix R_CCW and origin translation,
// find maximum extents, and return the resulting AABB
// DEBUG: must use local-space bounds for accurate size and position
eBounds GetAABBFromIsometricRotation(const eBounds & bounds, const eVec2 & origin) {
	// 45 degrees CCW about z-axis
	static const float cos45 = SDL_sqrtf(2) * 0.5f;
	static const eVec2 R_CCW[2] = {	eVec2(cos45, cos45),
									eVec2(-cos45, cos45)};

	eVec2 center, extents;
	eVec2 rotatedCenter, rotatedExtents;
	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;

	for (int i = 0; i < 2; i++) {
		rotatedExtents[i] = abs(extents[0] * R_CCW[0][i]) +
							abs(extents[1] * R_CCW[1][i]);

		// FIXME: if using local-space then bounds center is 0,0 and will remain so
		// therefore this step is overkill
		rotatedCenter[i] = origin[i];
		rotatedCenter[i] +=	(center[0] * R_CCW[0][i]) +
							(center[1] * R_CCW[1][i]);
	}

	eBounds aabb;
	aabb[0] = rotatedCenter - rotatedExtents;
	aabb[1] = rotatedCenter + rotatedExtents;
	return aabb;
}

typedef struct OBB_s {
	eVec2 center;		// world-space center
	eVec2 axes[2];		// unit-length locally oriented x and y axes
	eVec2 halfSize;		// positive distance along local x and y axes
} OBB_t;

// DEBUG: edges must be perpendicular
// and corner must be their point of intersection
// TODO: this should be a constructor
OBB_t GetOBBFromEdges(const eVec2 & corner, const eVec2 * edges) {
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

// converts AABB to OBB
// TODO: this should be a constructor
OBB_t GetOBBFromAABB(const eBounds & bounds) {
	OBB_t obb;
	obb.center = (bounds[0] + bounds[1]) * 0.5f;
	obb.halfSize = bounds[1] - obb.center;
	obb.axes[0] = eVec2(1.0f, 0.0f);
	obb.axes[1] = eVec2(0.0f, 1.0f);
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
//	map.Draw();
// BEGIN FREEHILL DEBUG map area draw test
	// DEBUG: correct cells area on the isometric map that need to be drawn
	auto & absBounds = camera.CollisionModel().AbsBounds();
	auto & localBounds = camera.CollisionModel().LocalBounds();
	std::array<eVec2, 4> obbPoints;
	ToPoints(absBounds, obbPoints.data());
	auto origin = absBounds.Center();
	eMath::IsometricToCartesian(origin.x, origin.y);
//	RotatePoint(origin, 45, vec2_zero);
	for (int i = 0; i < obbPoints.size(); i++) {			// FIXME(performance): the next camOBB point means this only happens once	
//		eMath::IsometricToCartesian(obbPoints[i].x, obbPoints[i].y);
		RotatePoint(obbPoints[i], 45.0f, origin);			// FIXME: this needs a better rotation origin
	}

	eVec2 camEdges[2] = { obbPoints[1] - obbPoints[0], obbPoints[3] - obbPoints[0] };
	OBB_t camOBB = GetOBBFromEdges(obbPoints[0], camEdges);		// FIXME(performance): save this with eCamera and just translate it parallel

//	eBounds fitAABB = GetAABBFromIsometricRotation(localBounds, camOBB.center);
	eBounds fitAABB = GetAABBFromPoints(obbPoints);
	static std::vector<eGridCell *> areaCells;
	eCollision::GetAreaCells(fitAABB, areaCells);			// FIXME(performance): if the camera doesn't move, then areaCells won't change
															// use that temporal coherency to avoid re-getting the cells
															// and/or use spatial coherency (assuming a SetOrigin didn't happen)
															// to only add/prune the new/old cells because most will be the same
	for (auto && cell : areaCells) {		
		OBB_t cellOBB = GetOBBFromAABB(cell->AbsBounds());	// FIXME(performance): this could be stored INSTEAD of cell.AbsBounds 
															// because only ForwardCollisionTest uses that for broadphaseing
															// so AABBFromOBB for that may be better
		if (OBBOBBTest(cellOBB, camOBB)) {
			cell->Draw();
		}
	}
	areaCells.clear();
// END FREEHILL DEBUG draw order 

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
