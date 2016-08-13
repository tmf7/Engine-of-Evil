#ifndef ENTITY_H
#define ENTITY_H

#include "Game.h"

class Game;
class Map;

class Entity {

private:

	void CollisionCheck(bool horizontal, bool vertical);
	void CheckSight();
	void CheckTouch(bool self, bool horizontal, bool vertical);
	void PrintSensors();

	Game* game;
	SDL_Surface* sprite;
	SDL_Rect sight;				// TODO: make sight and touch part of organized structs?
	int knownMap[MAX_MAP_SIZE][MAX_MAP_SIZE];
	int x;
	int y;
	unsigned int moveState;
	unsigned int oldMoveState;
	int	speed;
	int size;
	int sightRange;				// range of drawable visibility (fog of war)
	int touchRange;				// range beyond body to trigger touch sensors

	int firstFrame;
	int lastFrame;
	int currentFrame;
	int frameDelay;
	int frameDelayCount;

	unsigned int localTouch;	// on-sprite sensors
	unsigned int oldTouch;		// off-sprite sensors
	unsigned int touch;			// off-sprite sensors
	unsigned int watch_touch;	// marks forward-sensors to watch given the moveState

	enum sensors {

		TOP_LEFT		= 1,
		TOP_RIGHT		= 2,
		RIGHT_TOP		= 4,
		RIGHT_BOTTOM	= 8,
		BOTTOM_RIGHT	= 16,
		BOTTOM_LEFT		= 32,
		LEFT_BOTTOM		= 64,
		LEFT_TOP		= 128

	};

	// move_state bits for direction and collision decisions
	enum movement {

		MOVE_LEFT	= 1,
		MOVE_RIGHT	= 2,
		MOVE_UP		= 4,
		MOVE_DOWN	= 8

	};

public:

	Entity();
	~Entity();

	bool Init(char fileName[], bool key, Game *const g);
	void Free();
	void Spawn();
	void Update();
	void Move();		// give this a broader functionality beyond pure AI movement (ie player input)
	int KnownMap(int row, int column);
};

#endif