#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>
#include <stdio.h>

#include "e_Math.h"
#include "e_Stack.h"
#include "Map.h"
#include "Entity.h"

#define MAX_WAYPOINTS 4096

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

	typedef struct waypoint_s {

		EvilStack<eVec2>		node;
		eVec2					point;
		bool					userDefined;		// to prevent overwriting
		int						timeOfAccess;		// in ms, oldest gets top overwriting priority

	} waypoint_t;

	waypoint_t waypointPool[MAX_WAYPOINTS];			// available memory for all entities' waypoints

	size_t				numActiveWaypoints;

public:

						Game();

	bool				Init();
	void				Shutdown(int error);
	bool				Run();
	void				DrawOutlineText(char* string, int x, int y, Uint8 r, Uint8 g, Uint8 b);
	SDL_Surface*		GetBuffer();
	Map*				GetMap();
	Entity*				GetEntities();
	waypoint_t*			GetNewWaypoint();
};

#endif /* GAME_H */