#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "Definitions.h"
#include "Math.h"
#include "SpatialIndexGrid.h"

class Game;

class Map {
private:

	Game *			game;
	SDL_Surface *	tileSet;						// FIXME(?): includes background?
	game_map_t		tileMap;

	void			MoveCamera();
	
public:

	enum tileType {
		COLLISION_TILE,
		TRAVERSABLE_TILE,
		RANDOM_TILE
	};

	struct {
		eVec2 position;
		float speed;
	} camera;

						Map();

	bool				Init(char fileName[], Game * const game, int maxRows, int maxCols);
	bool				IsValid(const eVec2 & point);
	void				Free();
	void				Update();
	void				BuildTiles(const int type);
	void				ToggleTile(const eVec2 & point);

	SDL_Surface * 		GetTile(int tileNumber);
	const game_map_t &	TileMap() const;
};

inline Map::Map() {
}

// TODO: input is which frame/tile number from the map's tileset to get, returns an SDL_Surface* of that
inline SDL_Surface * Map::GetTile(int tileNumber) {
	return tileSet;
}

inline const game_map_t & Map::TileMap() const {
	return tileMap;
}

#endif /* EVIL_MAP_H */
