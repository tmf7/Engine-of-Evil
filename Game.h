#ifndef GAME_H
#define GAME_H

#include "Definitions.h"
#include "Renderer.h"
#include "Math.h"
#include "Map.h"
#include "Entity.h"

class Game {
private:

	Entity				entities[MAX_ENTITIES];
	Map					map;
	Renderer			renderer;
	int					numEntities;

	void				FreeAssets();

public:

	enum ErrorCode {
		SDL_ERROR,
		RENDERER_ERROR,
		MAP_ERROR,
		ENTITY_ERROR,
		INIT_SUCCESS = -1
	};

						Game();

	ErrorCode			Init();
	void				Shutdown(ErrorCode error);
	bool				Run();

	Renderer &			Renderer();
	Map &				Map();
	Entity *			Entity(int entityID);
};

inline Game::Game() : numEntities(NULL) {
}

inline Renderer & Game::Renderer() {
	return renderer;
}

inline Map & Game::Map() {
	return map;
}

// user must check for nullptr return value
inline Entity * Game::Entity(int entityID) {
	return (entityID >= 0 && entityID <= numEntities ? &entities[entityID] : nullptr);
}

#endif /* GAME_H */