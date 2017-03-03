#ifndef EVIL_GAME_H
#define EVIL_GAME_H

#include "Definitions.h"
#include "Renderer.h"
#include "ImageManager.h"
#include "ImageTilerManager.h"
#include "Vector.h"
#include "Map.h"
#include "Entity.h"
#include "Camera.h"
#include "AI.h"
#include "Input.h"

class eGame {
public:

	struct {
		bool	GOAL_WAYPOINTS		= true;
		bool	TRAIL_WAYPOINTS		= true;
		bool	COLLISION			= true;
		bool	KNOWN_MAP_DRAW		= true;
		bool	KNOWN_MAP_CLEAR		= true;
		bool	FRAMERATE			= true;
		bool	A_STAR_PATH			= true;
		bool	GRID_OCCUPANCY		= true;
	} debugFlags;

	enum ErrorCode {
		SDL_ERROR,
		INPUT_ERROR,
		RENDERER_ERROR,
		IMAGE_MANAGER_ERROR,
		TILER_MANAGER_ERROR,
		MAP_ERROR,
		ENTITY_ERROR,
		INIT_SUCCESS = -1
	};

								eGame();

	ErrorCode					Init();
	void						Shutdown(ErrorCode error);
	bool						Run();

	eInput &					GetInput();
	eRenderer &					GetRenderer();
	eImageManager &				GetImageManager();
	eImageTilerManager &		GetImageTilerManager();
	eCamera &					GetCamera();
	eMap &						GetMap();
	std::shared_ptr<eEntity>	GetEntity(int entityID);

	// frame-rate metrics
	void						SetFixedFPS(const Uint32 newFPS);
	Uint32						GetFixedFPS() const;
	Uint32						GetFixedTime() const;
	Uint32						GetDynamicFPS() const;
	Uint32						GetDeltaTime() const;
	void						DrawFPS();

private:

	void						FreeAssets();

private:

	std::vector<std::shared_ptr<eEntity>>	entities;
	eHashIndex								entityHash;

	eInput						input;
	eMap						map;
	eRenderer					renderer;
	eImageManager				imageManager;
	eImageTilerManager			imageTilerManager;
	eCamera						camera;

	const Uint32				defaultFPS = 60;
	Uint32						fixedFPS;			// constant framerate
	Uint32						frameTime;			// constant framerate governing time interval (depends on FixedFPS)
	Uint32						deltaTime;			// actual time a frame takes to execute
};

extern eGame	game;								// one instance used by all objects

//*****************
// VerifyRead 
// global file I/O utility
// tests for unrecoverable read error or improperly formatted file
//*****************
inline bool VerifyRead(std::ifstream & read) {
	if (read.bad() || read.fail()) {
		read.clear();
		read.close();
		return false;
	}
	return true;
}

//****************
// eGame::eGame
//****************
inline eGame::eGame() {
	entities.reserve(MAX_ENTITIES);
	SetFixedFPS(defaultFPS);
}

//****************
// eGame::GetInput
//****************
inline eInput & eGame::GetInput() {
	return input;
}

//****************
// eGame::GetRenderer
//****************
inline eRenderer & eGame::GetRenderer() {
	return renderer;
}

//*****************
// eEditor::GetImageManager
//*****************
inline eImageManager & eGame::GetImageManager() {
	return imageManager;
}

//*****************
// eEditor::GetImageTilerManager
//*****************
inline eImageTilerManager & eGame::GetImageTilerManager() {
	return imageTilerManager;
}

//****************
// eGame::GetCamera
//****************
inline eCamera & eGame::GetCamera() {
	return camera;
}

//****************
// eGame::GetMap
//****************
inline eMap & eGame::GetMap() {
	return map;
}

//****************
// eGame::GetEntity
// DEBUG: ASSERT (entityID >= 0 && entityID <= numEntities)
//****************
inline std::shared_ptr<eEntity> eGame::GetEntity(int entityID) {
	return entities[entityID];
}

//****************
// eGame::SetFixedFPS
//****************
inline void eGame::SetFixedFPS(const Uint32 newFPS) {
	fixedFPS = newFPS;
	frameTime = 1000 / fixedFPS;
}

//****************
// eGame::GetFixedFPS
//****************
inline Uint32 eGame::GetFixedFPS() const {
	return fixedFPS;
}

//****************
// eGame::GetFixedTime
//****************
inline Uint32 eGame::GetFixedTime() const {
	return frameTime;
}

//****************
// eGame::GetDynamicFPS
//****************
inline Uint32 eGame::GetDynamicFPS() const {
	if (deltaTime)
		return 1000 / deltaTime;
	else
		return fixedFPS;
}

//****************
// eGame::GetDeltaTime
//****************
inline Uint32 eGame::GetDeltaTime() const {
	return deltaTime;
}

#endif /* EVIL_GAME_H */