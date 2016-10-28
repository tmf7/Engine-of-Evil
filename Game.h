#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>
#include <stdio.h>

#define BIT(a) (1<<a)
typedef unsigned char byte_t;

#include "Math.h"
#include "Deque.h"
#include "SpatialIndexGrid.h"
#include "Map.h"
#include "Entity.h"

class Map;
class Entity;

class Game {

private:

	void				FreeAssets();

	SDL_Surface*		backbuffer;
	SDL_Surface*		clear;
	SDL_Window*			window;
	TTF_Font*			font;

	Map map;
	Entity entities;	// TODO: make this a fixed size array

public:

						Game();

	bool				Init();
	void				Shutdown(int error);
	bool				Run();
	void				DrawOutlineText(char* string, int x, int y, Uint8 r, Uint8 g, Uint8 b);
	SDL_Surface*		GetBuffer();
	Map*				GetMap();
	Entity*				GetEntities();
};

#endif /* GAME_H */