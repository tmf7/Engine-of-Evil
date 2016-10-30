#ifndef GAME_H
#define GAME_H

#include "Math.h"
#include "Map.h"
#include "Entity.h"

//class Map;
//class Entity;
//class eVec2;

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
	SDL_Surface *		GetBuffer();
	Map *				GetMap();
	Entity *			GetEntities();
};

inline Game::Game() {
}

inline SDL_Surface* Game::GetBuffer() {
	return backbuffer;
}

inline Map * Game::GetMap() {
	return &map;
}

inline Entity * Game::GetEntities() {
	return &entities;
}

#endif /* GAME_H */