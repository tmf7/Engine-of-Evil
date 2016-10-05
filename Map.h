#ifndef MAP_H
#define MAP_H

#include "Game.h"

#define MAX_MAP_SIZE 256

// knownMap values
#define VISIBLE_TILE 2
#define VISITED_TILE 1
#define NONVISIBLE_TILE 0

// tileMap values
#define SOLID_TILE 1
#define NONSOLID_TILE 0

#define INVALID_TILE -1

class Game;
class Entity;

class Map {

private:

	void			CheckInput();

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
