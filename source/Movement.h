#ifndef EVIL_MOVEMENTPLANNER_H
#define EVIL_MOVEMENTPLANNER_H

#include "Definitions.h"
#include "Deque.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"

class eEntity;

typedef unsigned char byte_t;

template <class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> byte_map_t;


//*************************************************
//				eMovementPlanner
// updates owner's velocity to avoid collision
// and pathfind to goal waypoints
// TODO: inherit from an eComponent class
// that can be used by an eGameObject
//*************************************************
class eMovementPlanner : public eClass {
public:

						eMovementPlanner(const float movementSpeed);
	virtual				~eMovementPlanner() = default;

	void				Init(eEntity * const owner);
	void				Update();
	void				DebugDraw();							
	void				AddUserWaypoint(const eVec2 & waypoint);
	const byte_map_t &	KnownMap() const;
	void				ClearTrail();
	void				TogglePathingState();
	float				Speed() const;

	// debugging
	void				DrawGoalWaypoints();
	void				DrawTrailWaypoints();
	void				DrawKnownMap() const;

	virtual int			GetClassType() const override { return CLASS_MOVEMENT; }

private:

	// values for byte_map_t knownMap;
	typedef enum {
		UNKNOWN_TILE,
		VISITED_TILE
	} tileState_t ;

	// used to decide on a new movement direction
	typedef struct decision_s {
		eVec2			vector		= vec2_zero;
		float			stepRatio	= 0.0f;		// ratio of valid steps to those that land on previously unvisited tiles
		float			validSteps	= 0.0f;		// collision-free steps that could be taken along the vector
	} decision_t;

	typedef enum {
		MOVETYPE_NONE,		// TODO: actually integrate this
		MOVETYPE_GOAL,		// waypoint tracking
		MOVETYPE_TRAIL		// waypoint tracking
	} movementType_t;
	
	typedef enum {
		PATHTYPE_NONE,		// TODO: actually integrate this
		PATHTYPE_COMPASS,
		PATHTYPE_WALL
	} pathfindingType_t;

private:

	eEntity *			owner;					// back-pointer to user

	byte_map_t			knownMap;				// tracks visited tiles 
	movementType_t		moveState;
	pathfindingType_t	pathingState;

	float				maxMoveSpeed;
	float				collisionRadius;		// circular collision radius for prediction when using line of sight
	float				goalRange;				// acceptable range to consider the goal waypoint reached

	eDeque<eVec2>		trail;					// AI-defined waypoints for effective backtracking
	eDeque<eVec2>		goals;					// User-defined waypoints as terminal destinations
	eVec2 *				currentWaypoint;		// simplifies switching between the deque being tracked

	decision_t			forward;				// currently used movement vector
	decision_t			left;					// perpendicular to forward.vector counter-clockwise
	decision_t			right;					// perpendicular to forward.vector clockwise

	byte_t *			previousTile;			// most recently exited valid tile
	byte_t *			currentTile;			// tile at the entity's origin
	byte_t *			lastTrailTile;			// tile on which the last trail waypoint was placed (prevents redundant placement)

	// pathfinding (wall-follow)
	decision_t *		wallSide;				// direction to start sweeping from during PATHTYPE_WALL

	bool				moving;

private:

	// pathfinding (general)
	void				Move();
	bool				CheckVectorPath(decision_t & along);
	void				CheckWalls(float * bias);
	void				UpdateWaypoint(bool getNext = false);

	// pathfinding (wall-follow)
	void				WallFollow();

	// pathfinding (compass)
	void				CompassFollow();
	bool				CheckTrail();
	void				UpdateKnownMap();
	void				StopMoving();
};

//*************
// eMovementPlanner::KnownMap
//*************
inline const byte_map_t & eMovementPlanner::KnownMap() const {
	return knownMap;
}

//*************
// eMovementPlanner::TogglePathingState
//*************
inline void eMovementPlanner::TogglePathingState() {
	pathingState = (pathingState == PATHTYPE_COMPASS ? PATHTYPE_WALL : PATHTYPE_COMPASS);
	moveState = MOVETYPE_GOAL;
}

//*************
// eMovementPlanner::Speed
//*************
inline float eMovementPlanner::Speed() const {
	return maxMoveSpeed;
}

#endif /* EVIL_MOVEMENTPLANNER_H */

