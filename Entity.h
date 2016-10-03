#ifndef ENTITY_H
#define ENTITY_H

#include "Game.h"

class Game;
class Map;

#define MAX_LOS_WEIGHT 5
#define ROTATION_INCREMENT 1.0f

class Entity {

private:

	void			CollisionCheck(bool horizontal, bool vertical);
	void			CheckFogOfWar();
	void			CheckLineOfSight();
	void			SetNextWaypoint();
	void			StopMoving();
	void			RemoveWaypoint();

	void			CheckTouch(bool self, bool horizontal, bool vertical);
	void			PrintSensors();

	void			DrawPixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b); // DEBUG: collision circle testing

	Game*			game;
	SDL_Surface*	sprite;
	SDL_Rect		sight;				// TODO: make sight and touch part of organized structs?
	int				knownMap[MAX_MAP_SIZE][MAX_MAP_SIZE];
	eVec2			spritePos;
	unsigned int	moveState;
	unsigned int	oldMoveState;
	int				speed;
	int				size;
	float			collisionRadius;		// circular collision radius for prediction when using line of sight
	float			waypointRange;			// acceptable range to snap sprite position to user-defined waypoint
	int				sightRange;				// range of drawable visibility (fog of war)
	int				touchRange;				// range beyond body to trigger touch sensors

	// TODO: incorperate animation handling instead of static images
	int				firstFrame;
	int				lastFrame;
	int				currentFrame;
	int				frameDelay;
	int				frameDelayCount;

	unsigned int	localTouch;		// on-sprite sensors
	unsigned int	oldTouch;		// off-sprite sensors
	unsigned int	touch;			// off-sprite sensors
	unsigned int	watch_touch;	// marks forward-sensors to watch given the moveState

	// these nodes function as the top of their stack for this entity
	EvilDeque<eVec2> trail;		// AI-defined AI movement tracking ( points into Game.h's waypointNodes array )
	EvilDeque<eVec2> goals;		// User-defined AI movement goals ( points into Game.h's waypointNodes array )
	eVec2 * currentWaypoint;		// simplifies switching between the deque being tracked

	eVec3			forward;				// currently used movement vector
	eVec3			left;					// perpendicular to forward_v counter-clockwise
	eVec3			right;					// perpendicular to forward_v clockwise
	eQuat			rotationQuat;

	bool			atWaypoint;
	bool			moving;

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

		MOVE_LEFT		= 1,			// wall-follow
		MOVE_RIGHT		= 2,			// wall-follow
		MOVE_UP			= 4,			// wall-follow
		MOVE_DOWN		= 8,			// wall-follow
		MOVE_TO_GOAL	= 16,			// waypoint tracking
		MOVE_TO_TRAIL	= 32			// waypoint tracking

	};

public:

					Entity();

	bool			Init(char fileName[], bool key, Game *const g);
	void			Free();
	void			Spawn();
	void			Update();
	void			Move();		// give this a broader functionality beyond pure AI movement (ie player input)
	int				GetKnownMap(int row, int column);
	void			AddWaypoint(const eVec2 & waypoint, bool userDefined);
	// TODO: add GetTrailTop() function for other entities to track this entity's path 
	// or something similar (ie the enemy doesn't know where the entity is going, only where its been
	// and picks up the trail where it last saw the entity
	int				GetCenterX();
	int				GetCenterY();
};

#endif /* ENTITY_H */