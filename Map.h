#ifndef MAP_H
#define MAP_H

#include "Game.h"

#define MAX_MAP_SIZE 256

class Game;
class Entity;

class Map {

private:

	void			CheckInput();

	Game *			game;
	SDL_Surface *	tileSet;					// includes background
	int				tileSize;
	int				width;						// in number of tiles
	int				height;						// in number of tiles
	int				tileMap[MAX_MAP_SIZE][MAX_MAP_SIZE];

	struct viewport {
		int x;
		int y;
		int speed;
	} camera;

public:

	Map();

	bool			Init(char fileName[], Game *const g, int xTiles, int yTiles);
	viewport *		GetCamera();
	int				GetTileSize();
	int				GetWidth();
	int				GetHeight();
	SDL_Surface *	GetTile(int tileNumber);
	int				GetMapIndex(int row, int column);
	void			Free();
	void			Update();
	void			BuildTiles();
};

#endif
