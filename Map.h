#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "Definitions.h"
#include "Math.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"

class Game;

class Map {
public:

	// TODO: make camera a separate (entity?) class
	struct {
		eBounds modelBounds;	// size/shape using model-coordinages
		eBounds absBounds;		// dimensions using map-coordinates
		eVec2 origin;			// center of bounds
		eVec2 velocity;
	} camera;

	enum tileType {
		COLLISION_TILE,
		TRAVERSABLE_TILE,
		RANDOM_TILE
	};

							Map();

	bool					Init(char fileName[], Game * const game, int maxRows, int maxCols);
	bool					IsValid(const eVec2 & point, bool ignoreCollision = false);
	void					Free();
	void					Update();
	void					BuildTiles(const int type);
	void					ToggleTile(const eVec2 & point);
	const game_map_t &		TileMap() const;

private:

	Game *					game;
	SDL_Surface *			tileSet;
	game_map_t				tileMap;

	void					MoveCamera();
};

//**************
// Map::Map
//**************
inline Map::Map() {
}

//**************
// Map::TileMap
//**************
inline const game_map_t & Map::TileMap() const {
	return tileMap;
}

#endif /* EVIL_MAP_H */
