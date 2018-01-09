/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_MOVEMENTPLANNER_H
#define EVIL_MOVEMENTPLANNER_H

#include "SpatialIndexGrid.h"
#include "CollisionModel.h"
#include "GridIndex.h"
#include "Bounds.h"
#include "Deque.h"

namespace evil {

class eEntity;

//*************************************************
//				eMovementPlanner
// updates owner's velocity to avoid collision
// and pathfind to goal waypoints
// DEBUG: owner must have an eCollisionModel for this to function
// TODO: give knownMap multiple layers to match eMap::tileMap,
// so this->owner can navigate them without blocking itself prematurely
// TODO: implement a eMovementStrategy to swap out the ::Move algorithm at runtime
// based on external decisions, and to make this a more flexible class
//*************************************************
class eMovementPlanner : public eComponent {

	ECLASS_DECLARATION(eMovementPlanner)
	ECOMPONENT_DECLARATION(eMovementPlanner)

public:

							eMovementPlanner(eGameObject * owner, float movementSpeed);

	void					DebugDraw();							
	void					AddUserWaypoint(const eVec2 & waypoint);
	void					ClearTrail();
	void					TogglePathingState();
	float					Speed() const;

	// debugging
	void					DrawGoalWaypoints();
	void					DrawTrailWaypoints();
	void					DrawKnownMap() const;

	virtual void			Update() override;
	virtual void			SetOwner(eGameObject * newOwner) override;

private:

	// values for known_map_t knownMap;
	 typedef enum : unsigned char {
		UNKNOWN_TILE,
		VISITED_TILE
	} tileState_t ;

	// if owner->collisionModel has been on a tile
	class eTileKnowledge : public eGridIndex {

			ECLASS_DECLARATION(eTileKnowledge)

		public:

			virtual void	Reset() override	{ eGridIndex::Reset(); value = UNKNOWN_TILE; }

		public:

			unsigned char value = UNKNOWN_TILE;
	};

	typedef eSpatialIndexGrid<eTileKnowledge, MAX_MAP_ROWS, MAX_MAP_COLUMNS> known_map_t;

	// used to decide on a new movement direction
	typedef struct decision_s {
		eVec2				vector		= vec2_zero;		// unit-vector movement direction
		float				stepRatio	= 0.0f;				// ratio of valid steps to those that land on previously unvisited tiles
		float				validSteps	= 0.0f;				// collision-free steps that could be taken along the vector
	} decision_t;

	typedef enum {
		MOVETYPE_NONE,										// TODO: actually integrate this
		MOVETYPE_GOAL,										// waypoint tracking
		MOVETYPE_TRAIL										// waypoint tracking
	} movementType_t;
	
	typedef enum {
		PATHTYPE_NONE,										// TODO: actually integrate this
		PATHTYPE_COMPASS,
		PATHTYPE_WALL
	} pathfindingType_t;

private:

	eCollisionModel &		ownerCollisionModel;			// always confirmed

	known_map_t				knownMap;						// tracks visited tiles 
	movementType_t			moveState;						// backtracking or heading to a goal
	pathfindingType_t		pathingState;					// method of deciding velocity

	float					maxMoveSpeed;
	float					goalRange;						// acceptable range to consider the goal waypoint reached

	eDeque<eVec2>			trail;							// *this defines waypoints for effective backtracking
	eDeque<eVec2>			goals;							// User-defined waypoints as terminal destinations
	eVec2 *					currentWaypoint = nullptr;		// simplifies switching between the deque being tracked

	decision_t				forward;						// currently used movement vector
	decision_t				left;							// perpendicular to forward.vector counter-clockwise
	decision_t				right;							// perpendicular to forward.vector clockwise

	eTileKnowledge *		previousTile	= nullptr;		// most recently exited valid tile
	eTileKnowledge *		currentTile		= nullptr;		// tile at the entity's origin
	eTileKnowledge *		lastTrailTile	= nullptr;		// tile on which the last trail waypoint was placed (prevents redundant placement)

	// pathfinding (wall-follow)
	decision_t *			wallSide		= nullptr;		// direction to start sweeping from during PATHTYPE_WALL

	bool					moving;							// set true if the controlled eCollisionModel has been given an non-zero velocity this frame
	bool					initialized		= false;		// set true the first time Update is called (during its call to Init), set false whenever SetOwner is called

private:

	void					Init();

	// pathfinding (general)
	void					Move();										// FIXME(?): intellisense loses the namespace scope after eTileKnowledge declaration so these fns have bad scope?
	bool					CheckVectorPath(decision_t & along);
	void					CheckWalls(float * bias);
	void					UpdateWaypoint(bool getNext = false);
	void					UpdateKnownMap();
	void					StopMoving();

	// pathfinding (wall-follow)
	void					WallFollow();

	// pathfinding (compass)
	void					CompassFollow();
	bool					CheckTrail();
};

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

}      /* evil */
#endif /* EVIL_MOVEMENTPLANNER_H */

