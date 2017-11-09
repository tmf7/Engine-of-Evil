#include "Movement.h"
#include "Game.h"

//***************
// eMovement::eMovement
//***************
eMovement::eMovement(const float movementSpeed)
	: maxMoveSpeed(movementSpeed),
	  goalRange(movementSpeed),
	  currentTile(nullptr),
	  previousTile(nullptr),
	  lastTrailTile(nullptr),
	  currentWaypoint(nullptr),
	  wallSide(nullptr),
	  pathingState(PATHTYPE_COMPASS),
	  moveState(MOVETYPE_GOAL) {
	auto & tileMap = game.GetMap().TileMap();
	knownMap.SetCellSize( tileMap.CellWidth(),
						  tileMap.CellHeight());
	knownMap.ClearAllCells();
}

//******************
// eMovement::StopMoving
//******************
void eMovement::StopMoving() {
	wallSide = nullptr;
	owner->collisionModel->Velocity().Zero();
	moving = false;
}

void eMovement::Init(eEntity * owner) {
	this->owner = owner;
	collisionRadius = owner->collisionModel->LocalBounds().Radius();
	currentTile		= &knownMap.Index(owner->collisionModel->Origin());
	previousTile	= currentTile;
	StopMoving();
}

//***************
// eMovement::Think
// selects and updates a pathfinding type (eg: waypoint+obstacle avoid, A* optimal path, wall follow, Area awareness, raw compass?, etc)
//***************
void eMovement::Think() {
	auto & ownerCollisionModel = owner->collisionModel;
	bool wasStopped = false;
	
	// only pathfind with a waypoint
	if (currentWaypoint != nullptr) {
		wasStopped = !moving; 
		Move();
		UpdateKnownMap();

		// drop a trail waypoint (but never in a deadend that stopped the entity last frame)
		if (moving && !wasStopped && moveState != MOVETYPE_TRAIL && lastTrailTile != currentTile) {
			trail.PushFront(ownerCollisionModel->Origin());
			lastTrailTile = currentTile;
		}

		// check if goal is close enough to stop
		if (ownerCollisionModel->Origin().Compare(*currentWaypoint, goalRange)) {
			StopMoving();
			UpdateWaypoint(true);
		}

		// finalize the move
		if (ownerCollisionModel->Velocity() != vec2_zero) {
			moving = true;
			ownerCollisionModel->UpdateOrigin();
		}
	}
}

//***************
// eMovement::Move
// sets the velocity based on path predictions
// FIXME: name this something better
//***************
void eMovement::Move() {
	switch(pathingState) {
		case PATHTYPE_COMPASS: CompassFollow(); return;
		case PATHTYPE_WALL: WallFollow(); return;
	}
}

//******************
// eMovement::WallFollow
// Determines the optimal movement vector to continue following a wall (as if a hand were placed on it)
// DEBUG: default movement towards waypoint; default search for walls right-to-left using the first-found
// TODO: incorperate knownMap/stepRatio usage, directional bias usage?, more stopping conditions, goal waypoint short-circuit (like CompassFollow)
//******************
void eMovement::WallFollow() {
	decision_t	test;					// vector tested for optimal travel decision
	float		rotationAngle;			// cumulative amount the testVector has rotated in its search
	eQuat *		rotationDirection;		// wallSide affects the sweep direction that constitutes around-front of the entity
	bool		wallFollowing;
	bool		nearWall;

	if (!moving) { 
		forward.vector = *currentWaypoint - owner->collisionModel->Origin();
		forward.vector.Normalize();
		moving = true;
	}

	CheckWalls(nullptr);
	if (wallSide != nullptr) {
		test = *wallSide;
		// initially sweep behind the entity to confirm wall location
		rotationDirection = wallSide == &left ? &rotateCounterClockwiseZ : &rotateClockwiseZ;
		wallFollowing = true;
	} else {
		test = forward;
		// initially sweep in front of entity if no wall is being followed
		rotationDirection = &rotateCounterClockwiseZ;
		wallFollowing = false;
	}

	nearWall = false;
	rotationAngle = 0.0f;
	while (rotationAngle < 360.0f) {
		CheckVectorPath(test);
		if (wallFollowing && nearWall && test.validSteps > 0.0f) { 
			forward = test;
			CheckWalls(nullptr);
			break;					
		} else if (wallFollowing && !nearWall && test.validSteps == 0.0f) {
			nearWall = true;
			// change to sweeping in front of entity for a path along the wall
			rotationDirection = wallSide == &left ? &rotateClockwiseZ : &rotateCounterClockwiseZ;
			rotationAngle = 0.0f;
		} else if (!wallFollowing && test.validSteps == 0.0f) {
			wallSide = right.vector * test.vector >= left.vector * test.vector ? &right : &left;
			// continue sweeping in front of entity for a path along the wall
			rotationDirection = wallSide == &left ? &rotateClockwiseZ : &rotateCounterClockwiseZ;
			test = *wallSide;
			rotationAngle = 0.0f;
			wallFollowing = true;
			continue;
		}
		test.vector = *rotationDirection * test.vector;
		rotationAngle += ROTATION_INCREMENT;
	}

	if (wallSide != nullptr && (!nearWall || rotationAngle >= 360.0f))
		StopMoving();
	else 
		owner->collisionModel->Velocity() = forward.vector * maxMoveSpeed;
	// moveState may have changed, track the correct waypoint
//	UpdateWaypoint();
}

//******************
// eMovement::CompassFollow
// Determines the optimal movement vector to reach the current waypoint
//******************
void eMovement::CompassFollow() {
	decision_t	waypoint;				// from the sprite to the next waypoint
	decision_t	test;					// vector tested for optimal travel decision
	decision_t	best;					// optimal movement
	float		rotationAngle;			// cumulative amount the testVector has rotated in its search
	float		maxRotation;			// disallow vectors that backtrack if already moving
	float		weight;					// net bias for a decision about a test
	float		bestWeight;				// highest net result of all modifications to validSteps
	float		bias[4] = { 2.0f, 1.0f, 1.05f, 1.1f };	// { waypoint, left, right, forward }
	auto &		ownerCollisionModel = owner->collisionModel;

	if (!moving) {
		test.vector = vec2_oneZero;
		maxRotation = 360.0f;
	} else {
		CheckWalls(bias);
		test = right;		// counter-clockwise sweep of 180 degree arc from right to left in the forward direction

//		if (moveState == MOVETYPE_GOAL)
		maxRotation = 180.0f;
//		else // moveState == MOVETYPE_TRAIL
//			maxRotation = 360.0f;				// FIXME/BUG: occasionally causes semi-permanent stuck-in-place jitter

		// stuck in a corner (look for the quickest and most waypoint-oriented way out)
		if ((forward.stepRatio == 0 && right.stepRatio == 0) || (forward.stepRatio == 0 && left.stepRatio == 0))
			*currentTile = VISITED_TILE;
	}

	waypoint.vector = *currentWaypoint - ownerCollisionModel->Origin();
	waypoint.vector.Normalize();

	bestWeight = 0.0f;
	rotationAngle = 0.0f;
	while (rotationAngle < maxRotation) {

		// check how clear the path is starting one step along it
		// and head straight for the waypoint if the test.vector path crosses extremely near it
		if (CheckVectorPath(test)) {
			if (CheckVectorPath(waypoint))
				forward = waypoint;
			else
				forward = test;

			// initilize the new left and right, and their validSteps that'll be used next frame
			CheckWalls(nullptr);
			ownerCollisionModel->Velocity() = forward.vector * maxMoveSpeed;
			return;
		}

		// FIXME/BUG: trail waypoint orbits or cannot attain sometimes (bad corner, whatever)
		// SOMEWHAT fixed by putting a trail waypoint on each new tile (its never too far to navigate straight back to)
		// SOMEWHAT fixed by modulating speed based on waypoint proximity, except backtracking looked weird so that was removed
		// SOLUTION(?): count how many steps its been on the current tile, if limit exceeded, then set currentTile to VISITED_TILE
		// SOLUTION(?): modulate speed if lingering on a tile & in given range of latest trail waypoint

		// give the path a bias to help set priority
		weight = test.validSteps;
		weight += (test.vector * waypoint.vector) * bias[0];
		if (moving) {
			weight += (test.vector * left.vector) * bias[1];
			weight += (test.vector * right.vector) * bias[2];
			weight += (test.vector * forward.vector) * bias[3];
		}

		// more new tiles always beats better overall weight
		if (test.stepRatio > best.stepRatio) {
			bestWeight = weight;
			best = test;
		} else if (test.stepRatio == best.stepRatio && weight > bestWeight) {
			bestWeight = weight;
			best = test;
		}
		test.vector = rotateCounterClockwiseZ * test.vector;
		rotationAngle += ROTATION_INCREMENT;
	}

	if (moveState == MOVETYPE_GOAL && best.stepRatio == 0) {	// deadlocked, begin deadend protocols (ie follow the trail now)
		StopMoving();
		moveState = MOVETYPE_TRAIL;
	} else if (moveState == MOVETYPE_TRAIL && best.stepRatio > 0) {
		StopMoving();
		moveState = MOVETYPE_GOAL;
	} else if (moveState == MOVETYPE_TRAIL && best.validSteps == 0) {
		StopMoving();
	} else {
		forward = best;
		// initilize the new left and right, and their validSteps that'll be used next frame
		CheckWalls(nullptr);
		ownerCollisionModel->Velocity() = forward.vector * maxMoveSpeed;
	}
	// moveState may have changed, track the correct waypoint
	UpdateWaypoint();
}

//******************
// eMovement::CheckVectorPath
// determines the state of the entity's position for the next few frames
// returns true if a future position using along is near the waypoint
//******************
bool eMovement::CheckVectorPath(decision_t & along) {
	static const float maxSteps = 5.0f;										// how many future steps to test

	auto & ownerCollisionModel = owner->collisionModel;
	auto & boundsCenter = ownerCollisionModel->AbsBounds().Center();
	float castLength = maxMoveSpeed * maxSteps;
	float nearestFraction = 1.0f;
	float mapEdgeFraction = 1.0f;

	auto & mapEdges = game.GetMap().EdgeColliders();
	for(int i = 0; i < mapEdges.size(); ++i) {
		float movingAway = mapEdges[i].second * along.vector;
		if (movingAway >= 0.0f) 
			continue;
		if (eCollision::MovingAABBAABBTest(ownerCollisionModel->AbsBounds(), along.vector, castLength, mapEdges[i].first, mapEdgeFraction) &&
			mapEdgeFraction < nearestFraction)
			nearestFraction = mapEdgeFraction;
	}

	along.validSteps = 0.0f;
	Collision_t collision;
	if (ownerCollisionModel->FindApproachingCollision(along.vector, castLength, collision) && 
		collision.fraction < nearestFraction) {
		nearestFraction = collision.fraction;
	} else {
		eVec2 endPoint = boundsCenter + along.vector * (nearestFraction * castLength);
		if (moveState == MOVETYPE_GOAL && endPoint.Compare(*currentWaypoint, goalRange)) {
			along.validSteps = floor(nearestFraction * maxSteps);
			return true;
		}
	}
	along.validSteps = floor(nearestFraction * maxSteps);

	// DEBUG: eCollision::GetAreaCells using along.vector grabs more cells 
	// than eMovement will when updating knownMap, so it's not used here
	eVec2 futureCenter = boundsCenter;
	float newSteps = 0.0f;
	for (int i = 0; i < along.validSteps; ++i) {
		if (knownMap.Index(futureCenter) == UNKNOWN_TILE)
			++newSteps;
		futureCenter += along.vector * maxMoveSpeed;
	}

	along.stepRatio = 0.0f;
	if (along.validSteps > 0.0f)
		along.stepRatio = newSteps / along.validSteps;

	return false;
}

//**************
// eMovement::CheckWalls
// assigns the vectors perpendicular to the forward vector
// and checks if the range along them has significantly changed
//**************
void eMovement::CheckWalls(float * bias) {
	static const float stepIncreaseThreshold = 2.0f;		// updated validSteps must be larger than the oldValidSteps + this to warrant a bias modification
	
	float oldLeftSteps = left.validSteps;
	float oldRightSteps = right.validSteps;
	left.vector.Set(-forward.vector.y, forward.vector.x);	// forward rotated 90 degrees counter-clockwise
	right.vector.Set(forward.vector.y, -forward.vector.x);	// forward rotated 90 degrees clockwise

	CheckVectorPath(forward);
	CheckVectorPath(left);
	CheckVectorPath(right);

	if (bias == nullptr)
		return;

	// bias[0] == waypoint bias remains unchanged because if * test.stepRatio; or * waypoint.stepRatio; then trail-less backtracking occurs (bad)
	bias[1] *= (left.validSteps >= oldLeftSteps + stepIncreaseThreshold) * left.stepRatio;		// left path opened
	bias[2] *= (right.validSteps >= oldRightSteps + stepIncreaseThreshold) * right.stepRatio;	// right path opened
	bias[3] *= !(forward.validSteps == 0.0f) * forward.stepRatio;								// forward path not closed
}

//******************
// eMovement::AddUserWaypoint
// TODO: allow other entities to become waypoints (that move)
// use a separate eVec2 * target = &targetOrigin; to set currentWaypoint during MOVETYPE_GOAL
//******************
void eMovement::AddUserWaypoint(const eVec2 & waypoint) {
	static std::vector<Collision_t> collisions;		// FIXME(~): make this a private data member instead of per-fn, if more than one fn uses it

	collisions.clear();		// DEBUG: lazy clearing
	eBounds waypointBounds = owner->collisionModel->LocalBounds() + waypoint;
	if(!eCollision::AABBContainsAABB(game.GetMap().AbsBounds(), waypointBounds) ||
		eCollision::BoxCast(collisions, waypointBounds, vec2_zero, 0.0f))
		return;

	goals.PushFront(waypoint);
	UpdateWaypoint();
}

//******************
// eMovement::UpdateWaypoint
//******************
void eMovement::UpdateWaypoint(bool getNext) {
	switch (moveState) {
		case MOVETYPE_GOAL: {
			if (getNext && !goals.IsEmpty()) {
				goals.PopBack();
				trail.Clear();
			}
			CheckTrail();
			if (!goals.IsEmpty()) {
				currentWaypoint = &goals.Back()->Data();
				return;
			}
			currentWaypoint = nullptr;
			return;
		}
		case MOVETYPE_TRAIL: {
			if (getNext && !trail.IsEmpty())
				trail.PopFront();
			if (!CheckTrail()) {
				currentWaypoint = &trail.Front()->Data();
				return;
			} else if (!goals.IsEmpty()) {
				moveState = MOVETYPE_GOAL;
				currentWaypoint = &goals.Back()->Data();
				return;
			}
			currentWaypoint = nullptr;
			return;
		}
		default: {		// DEBUG: currently for PATHTYPE_WALL
						// TODO: have PATHTYPE_WALL pay attention to knownMap and trail waypoints too
			if (getNext && !goals.IsEmpty()) {
				goals.PopBack();
				trail.Clear();
			}
			CheckTrail();
			if (!goals.IsEmpty()) {
				currentWaypoint = &goals.Back()->Data();
				return;
			}
			currentWaypoint = nullptr;
			return;
		}
	}
}

//******************
// eMovement::ClearTrail
// resets the knownMap so on the next UpdateKnownMap
// all trail waypoints are removed
//******************
void eMovement::ClearTrail() {
	knownMap.ClearAllCells();
	lastTrailTile = nullptr;
}

//******************
// eMovement::CheckTrail
// returns false if the entity should fresh-start goal pathfinding
//******************
bool eMovement::CheckTrail() {
	if (trail.IsEmpty()) {
		knownMap.ClearAllCells();
		lastTrailTile = nullptr;
		return true;
	}
	return false;
}

//******************
// eMovement::UpdateKnownMap
// marks the currentTile as VISITED_TILE, clears out un-needed trail waypoints,
// and resets tiles around the current goal waypoint to UNKNOWN_TILE
//******************
void eMovement::UpdateKnownMap() {
	byte_t * checkTile;
	int row, column;
	int startRow, startCol;
	int endRow, endCol;
	int tileResetRange;		// size of the box around the goal to set tiles to UNKNOWN_TILE

	// mark the tile to help future movement decisions
	// DEBUG: only needs to be more than **half-way** onto a new tile
	// to set the currentTile as previousTile and VISITED_TILE,
	// instead of completely off the tile (via a full absBounds check against the eGridCell bounds)
	checkTile = &knownMap.Index(owner->collisionModel->Origin());
	if (checkTile != currentTile) {
		previousTile = currentTile;
		*previousTile = VISITED_TILE;
		currentTile = checkTile;
	}

	tileResetRange = 0;
	if (!CheckTrail()) {
		
		// solid-box of tiles at the tileResetRange centered on the current goal waypoint to reset the knownMap
		if (!goals.IsEmpty()) {
			tileResetRange = (int)((goals.Back()->Data() - owner->collisionModel->Origin()).Length() / (knownMap.CellWidth() * 2));

			// find the knownMap row and column of the current goal waypoint
			knownMap.Index(goals.Back()->Data(), row, column);

			// set initial bounding box top-left and bottom-right indexes within knownMap
			startRow = row - (tileResetRange / 2);
			startCol = column - (tileResetRange / 2);
			endRow = row + (tileResetRange / 2);
			endCol = column + (tileResetRange / 2);

			// snap bounding box rows & columns within range of the tileMap area
			if (startRow < 0)
				startRow = 0;
			if (startCol < 0)
				startCol = 0;

			if (endRow >= knownMap.Rows())
				endRow = knownMap.Rows() - 1;
			if (endCol >= knownMap.Columns())
				endCol = knownMap.Columns() - 1;

			// reset tiles within the bounding box
			row = startRow;
			column = startCol;
			while (row <= endRow) {
				knownMap.Index(row, column) = UNKNOWN_TILE;

				column++;
				if (column > endCol) {
					column = startCol;
					row++;
				}
			}
		}
	}

	// pop all trail waypoints that no longer fall on VISITED_TILEs
	while (!trail.IsEmpty()) {
		if (knownMap.Index(trail.Back()->Data()) == UNKNOWN_TILE)
			trail.PopBack();
		else
			break;
	}
}

//******************
// eMovement::DebugDraw
//******************
void eMovement::DebugDraw() {
	DrawGoalWaypoints();
	DrawKnownMap();
	DrawTrailWaypoints();
}

//******************
// eMovement::DrawGoalWaypoints
//******************
void eMovement::DrawGoalWaypoints() {
	eNode<eVec2> * iterator;
	eVec2 goalPoint;

	if (!game.debugFlags.GOAL_WAYPOINTS)
		return;

	for (iterator = goals.Back(); iterator != nullptr; iterator = iterator->Next()) {
		goalPoint = iterator->Data();
		goalPoint.SnapInt();
		eBounds goalBounds = eBounds(goalPoint).ExpandSelf(goalRange);
		game.GetRenderer().DrawIsometricRect(redColor, goalBounds, RENDERTYPE_DYNAMIC);
	}
}

//******************
// eMovement::DrawTrailWaypoints
//******************
void eMovement::DrawTrailWaypoints() {
	eNode<eVec2> * iterator;
	eVec2 trailPoint;

	if (!game.debugFlags.TRAIL_WAYPOINTS)
		return;

	for(iterator = trail.Front(); iterator != nullptr; iterator = iterator->Prev()) {
		trailPoint = iterator->Data();
		trailPoint.SnapInt();
		eBounds trailBounds = eBounds(trailPoint).ExpandSelf(4);
		game.GetRenderer().DrawIsometricRect(greenColor, trailBounds, RENDERTYPE_DYNAMIC);
	}
}

//******************
// eMovement::DrawKnownMap
// TODO: check collision/draw layers, and draw debug rects over visited (and visible) tiles instead of entire cells.
//******************
void eMovement::DrawKnownMap() const {
	auto & tileMap = game.GetMap().TileMap();
	auto & visibleCells = game.GetMap().VisibleCells();
	for (auto & cell : visibleCells) {
		if (knownMap.Index(cell->GridRow(), cell->GridColumn()) == VISITED_TILE)
			game.GetRenderer().DrawIsometricRect(pinkColor, cell->AbsBounds(), RENDERTYPE_DYNAMIC);
	}

}