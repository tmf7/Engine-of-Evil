#ifndef EVIL_AI_H
#define EVIL_AI_H

#include "Deque.h"
#include "Vector.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Entity.h"

typedef unsigned char byte_t;
template <class type, int rows, int columns>
class eSpatialIndexGrid;
typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> byte_map_t;

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

class eAI : public eEntity {
public:

						eAI();

	virtual bool		Spawn() override;
	virtual void		Think() override;
	virtual void		Draw() override;
	void				AddUserWaypoint(const eVec2 & waypoint);
	const byte_map_t &	KnownMap() const;
	bool				CheckFogOfWar(const eVec2 & point) const;

private:

	byte_map_t			knownMap;				// tracks visited tiles 
	movementType_t		moveState;
	pathfindingType_t	pathingState;

	float				collisionRadius;		// circular collision radius for prediction when using line of sight
	float				goalRange;				// acceptable range to consider the goal waypoint reached
	float				sightRange;				// range of drawable visibility (fog of war)

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
	int					maxSteps;				// number of steps at current speed to project along a potential path

private:

	// pathfinding (general)
	void				Move();
	bool				CheckVectorPath(eVec2 from, decision_t & along);
	void				CheckWalls(float * bias);
	void				UpdateWaypoint(bool getNext = false);

	// pathfinding (wall-follow)
	void				WallFollow();

	// pathfinding (compass)
	void				CompassFollow();
	bool				CheckTrail();
	void				UpdateKnownMap();
	void				StopMoving();
	
	// debugging
	void				DrawGoalWaypoints();
	void				DrawTrailWaypoints();
	void				DrawCollisionCircle() const;
	void				DrawKnownMap() const;
};

//***************
// eAI::eAI
//***************
inline eAI::eAI() {
	sightRange	= 128.0f;
	goalRange	= velocity.Length();
	maxSteps	= 5;
}

//*************
// eAI::KnownMap
//*************
inline const byte_map_t & eAI::KnownMap() const {
	return knownMap;
}

//******************
// eAI::StopMoving
//******************
inline void eAI::StopMoving() {
	wallSide = nullptr;
	velocity.Zero();
	moving = false;
}

#endif /* EVIL_AI_H */

