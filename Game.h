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

	struct{ // TODO: change these names to DEBUG_NAME_HERE
		bool	GOAL_WAYPOINTS		= true;
		bool	TRAIL_WAYPOINTS		= true;
		bool	COLLISION_CIRCLE	= true;
		bool	KNOWN_MAP_DRAW		= true;
		bool	KNOWN_MAP_CLEAR		= true;
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

	ErrorCode			Init();
	void				Shutdown(ErrorCode error);
	bool				Run();

	eInput &			GetInput();
	eRenderer &			GetRenderer();
	eImageManager &			GetImageManager();
	eImageTilerManager &	GetImageTilerManager();
	eCamera &			GetCamera();
	eMap &				GetMap();
	eEntity *			GetEntity(int entityID);

private:

	eEntity *			entities[MAX_ENTITIES];
	eInput				input;
	eMap				map;
	eRenderer			renderer;
	eImageManager		imageManager;
	eImageTilerManager	imageTilerManager;
	eCamera				camera;
	int					numEntities;

	void				FreeAssets();
};

extern Uint32	globalIDPool;						// globally unique identifier: each resource gets, regardless of copying, moving, or type
extern eGame	game;								// one instance used by all objects
extern eAI		boss;								// FIXME: temporary solution to using dynamic memory to create object instances
													// then downcast them to their base class (then type check when extracting/using)

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
// entityID >= 0 && entityID <= numEntities only
//****************
inline eEntity * eGame::GetEntity(int entityID) {
	return entities[entityID];
}

#endif /* EVIL_GAME_H */