#ifndef EVIL_AI_H
#define EVIL_AI_H

#include "Deque.h"
#include "Vector.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Entity.h"

// FIXME: make these public enums, or eliminate them altogether
// and add public bool Entity::HasVisited(row,column) {...} or the like
// knownMap values
#define VISITED_TILE 1		// FIXME: should be Entity::VISITED_TILE
#define UNKNOWN_TILE 0		// FIXME: should be Entity::UNKOWN_TILE

// used to decide on a new movement direction
typedef struct decision_s {
	eVec2			vector		= vec2_zero;
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
	bool			operator==(const sensors_s & that);
	bool			operator!=(const sensors_s & that);
} sensors_t;

//***************
// sensors_s::operator==
//***************
inline bool sensors_s::operator==(const sensors_s & that) {
	return (TOP_LEFT == that.TOP_LEFT			&&
			TOP_RIGHT == that.TOP_RIGHT			&&
			RIGHT_TOP == that.RIGHT_TOP			&&
			RIGHT_BOTTOM == that.RIGHT_BOTTOM	&&
			BOTTOM_RIGHT == that.BOTTOM_RIGHT	&&
			BOTTOM_LEFT == that.BOTTOM_LEFT		&&
			LEFT_BOTTOM == that.LEFT_BOTTOM		&&
			LEFT_TOP == that.LEFT_TOP				);
}

//***************
// sensors_s::operator!=
//***************
inline bool sensors_s::operator!=(const sensors_s & that) {
	return !(*this == that);
}

typedef enum {
	MOVE_LEFT,			// wall-follow
	MOVE_RIGHT,			// wall-follow
	MOVE_UP,			// wall-follow
	MOVE_DOWN,			// wall-follow
	MOVE_TO_GOAL,		// waypoint tracking
	MOVE_TO_TRAIL		// waypoint tracking
} movementType_t;

typedef enum {
	COMPASS_FOLLOW_PATH,
	WALL_FOLLOW_PATH
} pathfindingType_t;

class eAI : public eEntity {
public:

						eAI();

	bool				Spawn();
	virtual void		Think();
	void				Draw();
	void				AddUserWaypoint(eVec2 & waypoint);
	const ai_map_t &	KnownMap() const;

private:

	struct {
		sensors_t		ranged;					// off-sprite sensors
		sensors_t		oldRanged;				// off-sprite sensors from the previous frame
		sensors_t		local;					// on-sprite sensors
		sensors_t		oldLocal;				// on-sprite sensors from the previous frame
		int				reach;					// distance beyond bounding box to trigger sensors
		void			Clear() { ranged.Clear(); oldRanged.Clear(); local.Clear(); oldLocal.Clear(); }
	} touch;

	ai_map_t			knownMap;				// tracks visited tiles from the game_map_t tileMap; in Map class
												// FIXME: this becomes 256 MB of ram for 4096 entities
												// SOLUTIONs: 1) change the algorithm to not need location tracking
												// 2) change the tileMap to track visited entities at each cell
												// 3) only give this to AI entities (significantly fewer than MAX_ENTITIES)
												// 4) use heap memory to only allocate visited locations (use hashindex or hashmap, slows the query?)
	movementType_t		moveState;
	pathfindingType_t	pathingState;

	float				collisionRadius;		// circular collision radius for prediction when using line of sight
	float				goalRange;				// acceptable range to consider the goal waypoint reached
	float				sightRange;				// range of drawable visibility (fog of war)

	eDeque<eVec2, 50>	trail;					// AI-defined waypoints for effective backtracking
	eDeque<eVec2, 50>	goals;					// User-defined waypoints as terminal destinations
	eVec2				currentWaypoint;		// simplifies switching between the deque being tracked

	decision_t			forward;				// currently used movement vector
	decision_t			left;					// perpendicular to forward.vector counter-clockwise
	decision_t			right;					// perpendicular to forward.vector clockwise

	byte_t *			previousTile;			// most recently exited valid tile
	byte_t *			currentTile;			// tile at the entity's origin
	byte_t *			lastTrailTile;			// tile on which the last trail waypoint was placed (prevents redundant placement)

	bool				moving;
	int					maxSteps;				// number of steps at current speed to project along a potential path

private:

	bool				CheckFogOfWar(const eVec2 & point) const;		// FIXME: should this be public?

	// pathfinding (wall-follow)
	void				WallFollow();
	void				CheckTouch();
	void				CheckCollision();								// FIXME: this should belong to a collision class

	// pathfinding (compass)
	void				Move();	
	void				CompassFollow();
	bool				CheckVectorPath(eVec2 from, decision_t & along);
	void				CheckWalls(bool & leftOpen, bool & rightOpen, bool & forwardHit);
	bool				UpdateWaypoint(bool getNext = false);
	void				StopMoving();									// FIXME: should this be public?
	bool				CheckTrail();
	void				UpdateKnownMap();

	// debugging
	void				DrawGoalWaypoints() const;
	void				DrawTrailWaypoints() const;
	void				DrawCollisionCircle() const;
	void				DrawTouchSensors() const;
	void				DrawKnownMap() const;
	bool				CheckKnownMapCleared();
};

//***************
// eAI::eAI
//***************
inline eAI::eAI() {
	sightRange	= 128.0f;
	goalRange	= speed;
	touch.reach = 1;
	maxSteps	= 5;
}

//*************
// eAI::KnownMap
//*************
inline const ai_map_t & eAI::KnownMap() const {
	return knownMap;
}

//******************
// eAI::StopMoving
//******************
inline void eAI::StopMoving() {
	forward.vector.Zero();
	moving = false;
}

#endif /* EVIL_AI_H */

