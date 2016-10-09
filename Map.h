#ifndef MAP_H
#define MAP_H

#include "Game.h"

#define MAX_MAP_SIZE 256

// FIXME: make these private enums?
// tileMap values
#define TRAVERSABLE_TILE 1
#define COLLISION_TILE 0

#define INVALID_TILE -1

class Game;
class Entity;

class Map {

private:

	void			MoveCamera();

	Game *			game;
	SDL_Surface *	tileSet;						// includes background
	int				tileSize;
	int				mapRows;						// number of tiles vertically
	int				mapCols;						// number of tiles horizontally
	int				tileMap[MAX_MAP_SIZE][MAX_MAP_SIZE];

	struct viewport {
		int x;
		int y;
		int speed;
	} camera;

public:

	Map();

	bool			Init(char fileName[], Game * const game, int maxRows, int maxCols);
	viewport *		GetCamera();
	int				GetTileSize() const;
	int				GetRows() const;
	int				GetColumns() const;
	int				GetWidth() const;
	int				GetHeight() const;
	SDL_Surface *	GetTile(int tileNumber);
	int				GetIndexValue(int row, int column) const;
	int				GetIndexValue(const eVec2 & point) const;
	void			GetIndex(const eVec2 & point, int & row, int & column) const;
	bool			IsValid(const eVec2 & point) const;
	void			Free();
	void			Update();
	void			BuildTiles();
};

#endif
