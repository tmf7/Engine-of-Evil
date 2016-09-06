#ifndef ENTITY_H
#define ENTITY_H

#include "Game.h"

class Game;
class Map;

#define VectorCopy(a,b)	( b[0]=a[0], b[1]=a[1], b[2]=a[2] )
#define VectorClear(a) ( a[0]=0, a[1]=0, a[2]=0 )

class Entity {

private:

	typedef float vec3_t[3];
	
	struct quat_s {

		vec3_t vector;
		float scalar;

	};

	struct point_s {

		int x;
		int y;
	};

	void CollisionCheck(bool horizontal, bool vertical);
	void CheckFogOfWar();
	void CheckLineOfSight();

	void CheckTouch(bool self, bool horizontal, bool vertical);
	void PrintSensors();

	// vector math functions
	float DegreesToRadians(float degrees);
	int FastLength(point_s *a, point_s *b);
	void VectorNormalize(vec3_t a);
	float VectorNormalize2(point_s *a, point_s *b, vec3_t result);
	float DotProduct(vec3_t a, vec3_t b);
	void VectorScale(vec3_t a, float scale, vec3_t result);
	void CrossProduct(vec3_t a, vec3_t b, vec3_t result);
	void QuatProduct(quat_s *a, quat_s *b, quat_s *result);


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
	float collisionRadius;		// circular collision radius for prediction when using line of sight
	int sightRange;				// range of drawable visibility (fog of war)
	int touchRange;				// range beyond body to trigger touch sensors
	int waypointRange;			// acceptable range to snap sprite position to user-defined waypoint

	// TODO: incorperate animation handling instead of static images
	int firstFrame;
	int lastFrame;
	int currentFrame;
	int frameDelay;
	int frameDelayCount;

	unsigned int localTouch;	// on-sprite sensors
	unsigned int oldTouch;		// off-sprite sensors
	unsigned int touch;			// off-sprite sensors
	unsigned int watch_touch;	// marks forward-sensors to watch given the moveState

	point_s waypoints[3];		// maximum of 3 waypoints for testing pathfinding
	point_s losEndpoint;		// exact point at which the sprite restarts the movementVector update
	vec3_t movementVector;		// currently used movement vector
	int currentWaypoint;		// index of waypoint being tracked
	int userWaypoint;			// index of waypoint being added
	int maxWaypoint;			

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
	void AddWaypoint(int wx, int wy);
	int GetCenterX();
	int GetCenterY();
};

#endif