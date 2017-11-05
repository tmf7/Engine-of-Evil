#ifndef EVIL_MOVEMENT_H
#define EVIL_MOVEMENT_H

#include "Deque.h"
#include "Vector.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Entity.h"

typedef unsigned char byte_t;
template <class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> byte_map_t;

class eMovement {
public:

						eMovement(const float movementSpeed);

	void				Init(eEntity * const owner);
	void				Think();
	void				DebugDraw();
	void				AddUserWaypoint(const eVec2 & waypoint);		// TODO: ePlayerInput should call this
	const byte_map_t &	KnownMap() const;

	// debugging
	void				DrawGoalWaypoints();
	void				DrawTrailWaypoints();
//	void				DrawCompassSearchArc();
//	void				DrawCollisionNormals();			// TODO: move this to eEntity, or ePlayerInput, or eCollision (with a parameter)
	void				DrawKnownMap() const;

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

	eEntity *			owner;	

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
// eMovement::KnownMap
//*************
inline const byte_map_t & eMovement::KnownMap() const {
	return knownMap;
}

//******************
// eMovement::StopMoving
//******************
inline void eMovement::StopMoving() {
	wallSide = nullptr;
	owner->collisionModel->Velocity().Zero();
	moving = false;
}

#endif /* EVIL_MOVEMENT_H */

