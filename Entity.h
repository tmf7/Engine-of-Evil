#ifndef ENTITY_H
#define ENTITY_H

#include "Game.h"

/*
An entity will be any object in game, be it an AI, a Player, an inanimate object, 
or an object with a scripted movement

Entity should be capable of:
- providing collision handling data
- providing rendering data
- etc (? later ?)

AI should be capable of:
- processing user commands (orders)
- processing AI-Type-specific scripts (procedural waypoints and other actions)
- deciding how to move (objectives) (access to a/the map)
- etc (later)

Player should be capable of:
- sending commands to AI (be it an indirect command or direct control)
- etc (later)
*/

class Game;
class Map;

// FIXME: make these private static const ints
// movement decision values
#define MAX_SPEED 10
#define MAX_STEPS 5
#define STEP_INCRESE_THRESHOLD 2
#define FORWARD_CHANGE_THRESHOLD 0.9f
#define ROTATION_INCREMENT 1.0f
#define RIGHT_WALL_OPENED BIT(1)
#define LEFT_WALL_OPENED BIT(2)
#define FORWARD_WALL_HIT BIT(3)
#define RIGHT_BIAS 1.05f
#define LEFT_BIAS 1.0f
#define FORWARD_BIAS 1.1f
#define WAYPOINT_BIAS 2.0f

// FIXME: make these private enums
// and add public bool Entity::HasVisited(row,column) {...} or the like
// knownMap values
#define VISITED_TILE 1
#define UNKNOWN_TILE 0
//******************
// Entity
//******************
class Entity {
private:

	// used to decide on a new movement direction
	typedef struct decision_s {
		eVec2	vector		= ZERO_VEC2;
		int		validSteps	= 0;			// collision-free steps that could be taken along the vector
		float	stepRatio	= 0.0f;			// ratio of valid steps to those that land on previously unvisited tiles
	} decision_t;

	typedef struct sensors_s {

		bool		TOP_LEFT		: 1;
		bool		TOP_RIGHT		: 1;
		bool		RIGHT_TOP		: 1;
		bool		RIGHT_BOTTOM	: 1;
		bool		BOTTOM_RIGHT	: 1;
		bool		BOTTOM_LEFT		: 1;
		bool		LEFT_BOTTOM		: 1;
		bool		LEFT_TOP		: 1;
		void		Clear() { memset(this, 0, sizeof(*this)); }

	} sensors_t;

	enum movement {
		MOVE_LEFT,			// wall-follow
		MOVE_RIGHT,			// wall-follow
		MOVE_UP,			// wall-follow
		MOVE_DOWN,			// wall-follow
		MOVE_TO_GOAL,		// waypoint tracking
		MOVE_TO_TRAIL		// waypoint tracking
	};

	Game *				game;
	SDL_Surface *		sprite;
	int					knownMap[MAX_MAP_ROWS][MAX_MAP_COLUMNS];// matches 1:1 the size of the tileMap in Map.h and the 
	int					knownMapRows;							// matches 1:1 the mapRows in Map.h
	int					knownMapCols;							// matches 1:1 the mapCols in Map.h
	eVec2				spritePos;
	eVec2				spriteCenter;
	int					moveState;
	int					oldMoveState;
	int					speed;
	int					size;
	float				collisionRadius;			// circular collision radius for prediction when using line of sight
	float				goalRange;					// acceptable range to snap sprite position to user-defined waypoint
	int					sightRange;					// range of drawable visibility (fog of war)

	struct {
		sensors_t		ranged;					// off-sprite sensors
		sensors_t		oldRanged;				// off-sprite sensors
		sensors_t		local;					// on-sprite sensors
		int				reach;					// distance beyond bounding box to trigger sensors
		void			Clear() { ranged.Clear(); oldRanged.Clear(); local.Clear(); }
	} touch;

	Deque<eVec2>		trail;					// AI-defined waypoints for effective backtracking
	Deque<eVec2>		goals;					// User-defined waypoints as terminal destinations
	eVec2				currentWaypoint;		// simplifies switching between the deque being tracked

	decision_t			forward;				// currently used movement vector
	decision_t			left;					// perpendicular to forward_v counter-clockwise
	decision_t			right;					// perpendicular to forward_v clockwise
	eQuat				rotationQuat_Z;			// to rotate any vector about z-axis
	int *				currentTile;			// to track where the sprite has been more accurately
	int *				lastTrailTile;			// tile on which the last trail waypoint was placed (prevents redundant placement)

	bool				moving;

private:

	bool				CheckFogOfWar(const eVec2 & point) const;		// FIXME: should this be public

	// functions related to wall following protocols			// TODO: move them to an AI : Entity class
	void				WallFollow();
	void				CheckTouch(bool self);
	void				CheckCollision();	// FIXME: should this belong to the Map class?
	void				PrintSensors();

	// functions related to waypoint protocols					// TODO: move them to an AI : Entity class
	void				WaypointFollow();
	void				UpdateVector();
	bool				CheckVectorPath(eVec2 from, decision_t & along);
	void				CheckWalls(int & walls);
	bool				UpdateWaypoint(bool getNext = false);
	void				StopMoving();							// FIXME: should this be public?
	bool				CheckTrail();
	void				UpdateKnownMap();

	void				UpdatePosition();
	void				UpdateCenter();

	void				DrawPixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b); // DEBUG: collision circle testing
	
public:

						Entity();

	bool				Init(char fileName[], bool key, Game * const game);
	void				Free();
	void				Spawn();
	void				Update();
	void				(Entity::*Move)(void);					// TODO: move this to an AI : Entity class and/or make virtual

	// TODO: make a general Map class that holds a 2d array and performs various get/set functions
	// then have entity use a KnownMap (instead of repeating this indexing code here)
	// AND have the game class...something...
	int *		 		KnownMapIndex(const eVec2 & point);
	int					KnownMapValue(int row, int column) const;
	int					KnownMapValue(const eVec2 & point);

	void				AddUserWaypoint(const eVec2 & waypoint); // TODO: move this to an AI : Entity class
	void				SetPosition(const eVec2 & point);
	const eVec2 &		Center() const;
};

#endif /* ENTITY_H */