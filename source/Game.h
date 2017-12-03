#ifndef EVIL_GAME_H
#define EVIL_GAME_H

#include "ImageManager.h"
#include "AnimationManager.h"
#include "AnimationControllerManager.h"
#include "EntityPrefabManager.h"
#include "Map.h"
#include "Camera.h"
#include "Input.h"
#include "Player.h"

//*************************************************
//					eGame
// handles all sub-system loading, initialization, and unloading
//*************************************************
class eGame : public eClass {
public:

	struct {
		bool	GOAL_WAYPOINTS		= false;
		bool	TRAIL_WAYPOINTS		= false;
		bool	COLLISION			= true;
		bool	RENDERBLOCKS		= true;
		bool	KNOWN_MAP_DRAW		= false;
		bool	KNOWN_MAP_CLEAR		= true;
		bool	FRAMERATE			= true;
		bool	GRID_OCCUPANCY		= false;
	} debugFlags;

	enum ErrorCode {
		SDL_ERROR,
		INPUT_ERROR,
		RENDERER_ERROR,
		IMAGE_MANAGER_ERROR,
		ANIMATION_MANAGER_ERROR,
		ANIMATION_CONTROLLER_MANAGER_ERROR,
		ENTITY_PREFAB_MANAGER_ERROR,
		MAP_ERROR,
		INIT_SUCCESS = -1
	};

public:

											eGame();

	ErrorCode								Init();
	void									Shutdown(ErrorCode error);
	bool									Run();

	eInput &								GetInput();
	eRenderer &								GetRenderer();
	eImageManager &							GetImageManager();
	eAnimationManager &						GetAnimationManager();
	eAnimationControllerManager &			GetAnimationControllerManager();
	eEntityPrefabManager &					GetEntityPrefabManager();
	eCamera &								GetCamera();
	eMap &									GetMap();
	int										AddEntity(std::unique_ptr<eEntity> && entity);
	void									RemoveEntity(int entityID);
	std::unique_ptr<eEntity> &				GetEntity(int entityID);
	int										NumEntities() const;

	// frame-rate metrics
	void									SetFixedFPS(const Uint32 newFPS);
	Uint32									GetFixedFPS() const;
	Uint32									GetFixedTime() const;
	Uint32									GetDynamicFPS() const;
	Uint32									GetDeltaTime() const;
	Uint32									GetGameTime() const;
	void									DrawFPS();

	virtual int								GetClassType() const override { return CLASS_GAME; }

private:

	void									FreeAssets();

private:

	std::vector<std::unique_ptr<eEntity>>	entities;

	eInput									input;
	eMap									map;
	eRenderer								renderer;
	eImageManager							imageManager;
	eAnimationManager						animationManager;
	eAnimationControllerManager				animationControllerManager;
	eEntityPrefabManager					entityPrefabManager;
	eCamera									camera;
	ePlayer									player;

	const Uint32							defaultFPS = 60;
	Uint32									fixedFPS;			// constant framerate
	Uint32									frameTime;			// constant framerate governing time interval (depends on FixedFPS)
	Uint32									deltaTime;			// actual time a frame takes to execute (to the nearest millisecond)
	Uint32									gameTime;			// time elapsed since execution began (updated at the end of each frame)
};

extern eGame	game;								// one instance used by all objects

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
// eGame::GetImageManager
//*****************
inline eImageManager & eGame::GetImageManager() {
	return imageManager;
}

//*****************
// eGame::GetAnimationManager
//*****************
inline eAnimationManager & eGame::GetAnimationManager() {
	return animationManager;
}

//*****************
// eGame::GetAnimationControllerManager
//*****************
inline eAnimationControllerManager & eGame::GetAnimationControllerManager() {
	return animationControllerManager;
}


//*****************
// eGame::GetEntityPrefabManager
//*****************
inline eEntityPrefabManager & eGame::GetEntityPrefabManager() {
	return entityPrefabManager;
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
// eGame::RemoveEntity
// DEBUG: ASSERT (entityID >= 0 && entityID < numEntities)
//****************
inline void eGame::RemoveEntity(int entityID) {
	entities[entityID] = nullptr;
}

//****************
// eGame::GetEntity
// DEBUG: ASSERT (entityID >= 0 && entityID < numEntities)
//****************
inline std::unique_ptr<eEntity> & eGame::GetEntity(int entityID) {
	return entities[entityID];
}

//****************
// eGame::NumEntities
//****************
inline int eGame::NumEntities() const {
	return entities.size();
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

//****************
// eGame::GetGameTime
//****************
inline Uint32 eGame::GetGameTime() const {
	return gameTime;
}

#endif /* EVIL_GAME_H */