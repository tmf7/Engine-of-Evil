#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>
#include <stdio.h>

#include "Map.h"
#include "Entity.h"

class Map;
class Entity;

class Game {

private:

	void FreeAssets();

	SDL_Surface* backbuffer;
	SDL_Surface* clear;
	SDL_Window* window;
	TTF_Font* font;

	// TODO: add a functional linked list of entities?
	// or a fixed array given a maximum number of entities?

	Map map;
	Entity entities;		// currently only one intended entity in-game

public:

	Game();
	~Game();

	bool Init();
	void Shutdown(int error);
	bool Run();
	void DrawOutlineText(char* string, int x, int y, Uint8 r, Uint8 g, Uint8 b);
	SDL_Surface* GetBuffer();
	Map* GetMap();
	Entity* GetEntities();
};

#endif