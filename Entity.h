#ifndef ENTITY_H
#define ENTITY_H

#include "Definitions.h"
#include "Deque.h"
#include "Math.h"
#include "SpatialIndexGrid.h"
#include "Sprite.h"
#include "Bounds.h"

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


// FIXME: make these private static const ints
// movement decision values
#define MAX_SPEED 10.0f
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

// FIXME: make these public enums
// and add public bool Entity::HasVisited(row,column) {...} or the like
// knownMap values
#define VISITED_TILE 1		// FIXME: should be Entity::VISITED_TILE
#define UNKNOWN_TILE 0		// FIXME: should be Entity::UNKOWN_TILE

class eGame;

//******************
// eEntity
//******************
class eEntity {
public:

						eEntity();

	bool				Init(char filename[], bool key, eGame * const game);
	void				Spawn();
	void				Update();
	void				(eEntity::*Move)(void);					// TODO: move this to an AI : Entity class and/or make virtual

	void				AddUserWaypoint(const eVec2 & waypoint); // TODO: move this to an AI : Entity class
	void				SetOrigin(const eVec2 & point);

	const ai_map_t &	KnownMap() const;
	const eVec2 &		Origin() const;

private:

	// used to decide on a new movement direction
	typedef struct decision_s {
		eVec2			vector		= ZERO_VEC2;
		float			stepRatio	= 0.0f;		// ratio of valid steps to those that land on previously unvisited tiles
		int				validSteps	= 0;		// collision-free steps that could be taken along the vector
	} decision_t;

	typedef struct sensors_s {
		bool			TOP_LEFT		: 1;
		bool			TOP_RIGHT		: 1;
		bool			RIGHT_TOP		: 1;
		bool			RIGHT_BOTTOM	: 1;
		bool			BOTTOM_RIGHT	: 1;
		bool			BOTTOM_LEFT		: 1;
		bool			LEFT_BOTTOM		: 1;
		bool			LEFT_TOP		: 1;
		void			Clear() { memset(this, 0, sizeof(*this)); }
	} sensors_t;

	struct {
		sensors_t		ranged;					// off-sprite sensors
		sensors_t		oldRanged;				// off-sprite sensors
		sensors_t		local;					// on-sprite sensors
		int				reach;					// distance beyond bounding box to trigger sensors
		void			Clear() { ranged.Clear(); oldRanged.Clear(); local.Clear(); }
	} touch;

	enum movement {
		MOVE_LEFT,			// wall-follow
		MOVE_RIGHT,			// wall-follow
		MOVE_UP,			// wall-follow
		MOVE_DOWN,			// wall-follow
		MOVE_TO_GOAL,		// waypoint tracking
		MOVE_TO_TRAIL		// waypoint tracking
	};

	eGame *				game;
	eSprite				sprite;	
	eBounds				modelBounds;			// using model coordinates
	eBounds				absBounds;				// using map coordinates
	eVec2				origin;
	eVec2				oldOrigin;				// for use with collision response
	eVec2				velocity;
	float				speed;

	ai_map_t			knownMap;				// tracks visited tiles from the game_map_t tileMap; in Map class

	int					moveState;
	int					oldMoveState;

	float				collisionRadius;		// circular collision radius for prediction when using line of sight
	float				goalRange;				// acceptable range to snap sprite position to user-defined waypoint
	float				sightRange;				// range of drawable visibility (fog of war)

	eDeque<eVec2, 50>	trail;					// AI-defined waypoints for effective backtracking
	eDeque<eVec2, 50>	goals;					// User-defined waypoints as terminal destinations
	eVec2				currentWaypoint;		// simplifies switching between the deque being tracked

	decision_t			forward;				// currently used movement vector
	decision_t			left;					// perpendicular to forward.vector counter-clockwise
	decision_t			right;					// perpendicular to forward.vector clockwise
	eQuat				rotationQuat_Z;			// to rotate any vector about z-axis
	byte_t *			currentTile;			// to track where the sprite has been more accurately
	byte_t *			lastTrailTile;			// tile on which the last trail waypoint was placed (prevents redundant placement)

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

	void				UpdateOrigin();
};

//***************
// eEntity::eEntity
//***************
inline eEntity::eEntity() {
	speed = MAX_SPEED;
	modelBounds.ExpandSelf(8);	// 16 x 16 square with (0, 0) at its center
	sightRange = 128.0f;
	goalRange = speed;
	rotationQuat_Z.Set(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));
	touch.reach = 1;
}

//*************
// eEntity::UpdateOrigin
//*************
inline void eEntity::UpdateOrigin() {
	oldOrigin = origin;
	velocity = forward.vector * speed;
	origin += velocity;
	absBounds = modelBounds + origin;
}

//*************
// eEntity::SetOrigin
//*************
inline void eEntity::SetOrigin(const eVec2 & point) {
	oldOrigin = point;
	origin = point;
	absBounds = modelBounds + origin;
}

//*************
// eEntity::Origin
//*************
inline const eVec2 & eEntity::Origin() const {
	return origin;
}

//*************
// eEntity::KnownMap
//*************
inline const ai_map_t & eEntity::KnownMap() const {
	return knownMap;
}

#endif /* ENTITY_H */