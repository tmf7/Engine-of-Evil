#ifndef EVIL_GAME_H
#define EVIL_GAME_H

#include "Definitions.h"
#include "Renderer.h"
#include "ImageManager.h"
#include "Vector.h"
#include "Map.h"
#include "Entity.h"
#include "Camera.h"
#include "AI.h"
#include "Input.h"

class eGame {
public:

	struct{
		bool	GOAL_WAYPOINTS		= true;
		bool	TRAIL_WAYPOINTS		= true;
		bool	COLLISION_CIRCLE	= true;
		bool	TOUCH_SENSORS		= true;
		bool	KNOWN_MAP_DRAW		= true;
		bool	KNOWN_MAP_CLEAR		= true;
	} debugFlags;

	enum ErrorCode {
		SDL_ERROR,
		INPUT_ERROR,
		RENDERER_ERROR,
		MAP_ERROR,
		ENTITY_ERROR,
		INIT_SUCCESS = -1
	};

						eGame();

	ErrorCode			Init();
	void				Shutdown(ErrorCode error);
	bool				Run();

	eInput &			GetInput();
	eRenderer &			GetRenderer();
	eImageManager &		GetImageManager();
	eCamera &			GetCamera();
	eMap &				GetMap();
	eEntity *			GetEntity(int entityID);

private:

	eEntity *			entities[MAX_ENTITIES];
	eInput				input;
	eMap				map;
	eRenderer			renderer;
	eImageManager		imageManager;
	eCamera				camera;
	int					numEntities;

	void				FreeAssets();
};

extern eGame game;						// one instance used by all objects
extern eAI boss;						// FIXME: temporary solution to using dynamic memory to create object instances
										// then downcast them to their base class (then type check when extracting/using)

//****************
// eGame::eGame
//****************
inline eGame::eGame() : numEntities(0) {
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

//****************
// eGame::GetImageManager
//****************
inline eImageManager & eGame::GetImageManager() {
	return imageManager;
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
// entityID >= 0 && entityID <= numEntities only
//****************
inline eEntity * eGame::GetEntity(int entityID) {
	return entities[entityID];
}

#endif /* EVIL_GAME_H */