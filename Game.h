#ifndef GAME_H
#define GAME_H

#include "Definitions.h"
#include "Renderer.h"
#include "ImageManager.h"
#include "Math.h"
#include "Map.h"
#include "Entity.h"

class eGame {
public:

	enum ErrorCode {
		SDL_ERROR,
		RENDERER_ERROR,
		MAP_ERROR,
		ENTITY_ERROR,
		INIT_SUCCESS = -1
	};

						eGame();

	ErrorCode			Init();
	void				Shutdown(ErrorCode error);
	bool				Run();

	eRenderer &			Renderer();
	eImageManager &		ImageManager();
	eMap &				Map();
	eEntity *			Entity(int entityID);

private:

	eEntity				entities[MAX_ENTITIES];
	eMap				map;
	eRenderer			renderer;
	eImageManager		imageManager;
	int					numEntities;

	void				FreeAssets();
};

//****************
// eGame::eGame
//****************
inline eGame::eGame() : numEntities(0) {
}

//****************
// eGame::Renderer
//****************
inline eRenderer & eGame::Renderer() {
	return renderer;
}

//****************
// eGame::ImageManager
//****************
inline eImageManager & eGame::ImageManager() {
	return imageManager;
}

//****************
// eGame::Map
//****************
inline eMap & eGame::Map() {
	return map;
}

//****************
// eGame::Entity
// entityID >= 0 && entityID <= numEntities only
//****************
inline eEntity * eGame::Entity(int entityID) {
	return &entities[entityID];
}

#endif /* GAME_H */