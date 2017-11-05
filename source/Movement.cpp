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
	bool wasStopped;

// BEGIN FREEHILL DEBUG AI/player control
	auto & input = game.GetInput();
	auto & ownerCollisionModel = owner->collisionModel;
	auto & ownerVelocity = ownerCollisionModel->Velocity();
	if (input.MousePressed(SDL_BUTTON_LEFT))
		AddUserWaypoint(game.GetMap().GetMouseWorldPosition());

	if (input.KeyPressed(SDL_SCANCODE_M)) {
		pathingState = (pathingState == PATHTYPE_COMPASS ? PATHTYPE_WALL : PATHTYPE_COMPASS);
		moveState = MOVETYPE_GOAL;
	} 

	float xMove = maxMoveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_RIGHT) - input.KeyHeld(SDL_SCANCODE_LEFT));
	float yMove = maxMoveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_DOWN) - input.KeyHeld(SDL_SCANCODE_UP));

	if (SDL_fabs(xMove) > 0.0f || SDL_fabs(yMove) > 0.0f) {
		eVec2 moveInput(xMove, yMove);
		moveInput.Normalize();
		moveInput *= maxMoveSpeed;
		eMath::IsometricToCartesian(moveInput.x, moveInput.y);
		ownerVelocity.Set(moveInput.x, moveInput.y);
		CollisionResponseSlide();
	}
// END FREEHILL DEBUG AI/player control

	// only pathfind with a waypoint
	if (currentWaypoint != nullptr) {
		wasStopped = !moving; 
		Move();
		UpdateKnownMap();		// FIXME: there is a user-input controlled function in here, it will not be called if UpdateWaypoint returns false

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
		if (ownerVelocity != vec2_zero) {
			moving = true;
			ownerCollisionModel->UpdateOrigin();
		}
	}
}

//***************
// eMovement::FindApproachingCollision
// returns true and sets result to the nearest non-tangential collision along dir * length
// returns false and leaves result unmodified otherwise
// DEBUG: dir must be unit length
// TODO(?): move this to eCollisionModel
//***************
bool eMovement::FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const {
	static std::vector<Collision_t> collisions;		// FIXME: make this a private data member instead of per-fn

	collisions.clear();	// DEBUG: lazy clearing
	if(eCollision::BoxCast(collisions, owner->collisionModel->AbsBounds(), dir, length)) {
		for (auto & collision : collisions) {
			float movingAway = collision.normal * dir;
			float movingAwayThreshold = ((abs(collision.normal.x) < 1.0f && abs(collision.normal.y) < 1.0f) ? -0.707f : 0.0f); // vertex : edge
			if (movingAway >= movingAwayThreshold) {
				continue;
			} else {
				result = collision;
				return true;
			}
		}
	}
	return false;
}

//***************
// eMovement::CollisionResponseCorrection
// TODO(?): move this to eCollisionModel
//***************
void eMovement::CollisionResponseCorrection() {
	auto & ownerCollisionModel = owner->collisionModel;
	auto & ownerVelocity = ownerCollisionModel->Velocity();
	if (ownerVelocity == vec2_zero)
		return;	

	Collision_t collision;
	float length = ownerVelocity.Length();
	if(FindApproachingCollision(ownerVelocity / length, length, collision))
		ownerVelocity *= collision.fraction;
	ownerCollisionModel->UpdateOrigin();
}

//***************
// eMovement::CollisionResponseSlide
// TODO(?): move this to eCollisionModel
//***************
void eMovement::CollisionResponseSlide() {
	auto & ownerCollisionModel = owner->collisionModel;
	auto & ownerVelocity = ownerCollisionModel->Velocity();
	if (ownerVelocity == vec2_zero)
		return;

	Collision_t collision;
	eVec2 collisionTangent;
	float remainingFraction = 0.0f;
	float length = ownerVelocity.Length();

	if (FindApproachingCollision(ownerVelocity / length, length, collision)) {
		remainingFraction = 1.0f - collision.fraction;
		if (remainingFraction < 1.0f) {
			ownerVelocity *= collision.fraction;
		} else {
			// DEBUG: collisions sorted w/edges first for equal fractions
			collisionTangent = eVec2(-collision.normal.y, collision.normal.x);		// CCW 90 degrees
			float whichWay = collisionTangent * ownerVelocity;
			if (whichWay < 0)
				collisionTangent *= -1.0f;
		} 
	}

	if (remainingFraction == 1.0f) {
		float slide = ownerVelocity * collisionTangent;
		ownerVelocity = collisionTangent * slide;	
		if (FindApproachingCollision(collisionTangent, slide, collision))
			ownerVelocity *= collision.fraction;
	}
	ownerCollisionModel->UpdateOrigin();
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

	auto & ownerBounds = owner->collisionModel->AbsBounds();
	auto & boundsCenter = ownerBounds.Center();
	float castLength = maxMoveSpeed * maxSteps;
	float nearestFraction = 1.0f;
	float mapEdgeFraction = 1.0f;

	auto & mapEdges = game.GetMap().EdgeColliders();
	for(int i = 0; i < mapEdges.size(); ++i) {
		float movingAway = mapEdges[i].second * along.vector;
		if (movingAway >= 0.0f) 
			continue;
		if (eCollision::MovingAABBAABBTest(ownerBounds, along.vector, castLength, mapEdges[i].first, mapEdgeFraction) &&
			mapEdgeFraction < nearestFraction)
			nearestFraction = mapEdgeFraction;
	}

	along.validSteps = 0.0f;
	Collision_t collision;
	if (FindApproachingCollision(along.vector, castLength, collision) && 
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
// moving waypoints would need to have their info updated in the deque (hence pointers instead of copies)
// of course that could be a separate category altogether
// SOLUTION: if currentWaypoint == &targetOrigin, then this would pursue, until it needed to backtrack on its trail,
// so use a separate eVec2 * target to set currentWaypoint once MOVETYPE_GOAL resumes (if its non-nullptr)
//******************
void eMovement::AddUserWaypoint(const eVec2 & waypoint) {
	static std::vector<Collision_t> collisions;

	collisions.clear();		// DEBUG: lazy clearing
	eBounds waypointBounds = owner->collisionModel->LocalBounds() + waypoint;
	if(!eCollision::AABBContainsAABB(game.GetMap().AbsBounds(), waypointBounds) ||
		eCollision::BoxCast(collisions, waypointBounds, vec2_zero, 0.0f))		// FIXME: make a straight-aabb vs local contents test (w/ & w/o collisions)
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
// eMovement::CheckTrail
// returns false if the entity should fresh-start goal pathfinding
//******************
bool eMovement::CheckTrail() {
	eInput * input;

	input = &game.GetInput();
	if (trail.IsEmpty() || game.debugFlags.KNOWN_MAP_CLEAR && input->KeyPressed(SDL_SCANCODE_R)) {
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
		
		// solid-box of tiles at the tileResetRange centered on **the current goal waypoint** to to reset the knownMap
		// FIXME: this Length() call is very costly and shouldn't run each frame
		// SOLUTION: find a workaround for using the knownMap at all, and/or prevent the current "orbit" behavior
		// that the AI displays when attempting to attain a goal (ie sometimes there's an opening that's missed in favor
		// of maintaining the directional weight)
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
//	DrawCompassSearchArc();
//	DrawCollisionNormals();
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

/*
std::vector<Collision_t> collisionNormalsDrawTest;		// debugging hack
//******************
// eMovement::DrawCollisionNormals
// DEBUG: this was originally used and populated during the player-control-input logic
//******************
void eMovment::DrawCollisionNormals() {
	float normalDrawLength = 100.0f;
	float screenCoord = 250.0f;
	float multiOffset = 30.0f;
	std::vector<eVec2> points;
	for (int count = 0; count < collisionNormalsDrawTest.size(); ++count) {
		auto & normal = collisionNormalsDrawTest[count];
		points[0] = { screenCoord + (multiOffset * count), screenCoord };
		points[1] = { points[0].x + normal.x * normalDrawLength * 0.5f, points[0].y + normal.y * normalDrawLength * 0.5f };
		game.GetRenderer().DrawLines(redColor, points, RENDERTYPE_DYNAMIC);

		points[0] = points[1];
		points[1] = { points[0].x + normal.x * normalDrawLength * 0.5f, points[0].y + normal.y * normalDrawLength * 0.5f };
		game.GetRenderer().DrawLines(blueColor, points, RENDERTYPE_DYNAMIC);
	}
	collisionNormalsDrawTest.clear();
}

//******************
// eMovement::DrawCompassSearchArc
// draws the forward sweep angle checked for collision prediction in any-angle pathfinding
// FIXME: draw a partial circle in eRenderer instead
//******************
void eMovement::DrawCompassSearchArc() {
	eVec2 debugVector;
	eVec2 debugPoint;
	float rotationAngle;
	int pink[3] = { 255, 0, 255 };
	int blue[3] = { 0,0,255 };
	int * color;

	if (!game.debugFlags.COMPASS_SEARCH)		// TODO: not defined, possibly replace with debugFlags.PATHFINDING to cover all eMovement Debug Drawing
		return;

	// draws one pixel for each point on the current collision circle 
	if (moveState == MOVETYPE_GOAL)
		color = pink;
	else
		color = blue;

	debugVector = vec2_oneZero;
	rotationAngle = 0.0f;
	while (rotationAngle < 360.0f) {
		if (collisionModel.Velocity() * debugVector >= 0) {
			debugPoint = collisionModel.Origin() + (debugVector * collisionRadius) - game.GetCamera().CollisionModel().AbsBounds()[0];
			debugPoint.SnapInt();
			game.GetRenderer().DrawPixel(debugPoint, color[0], color[1], color[2]);
		}
		debugVector = rotateCounterClockwiseZ * debugVector;
		rotationAngle += ROTATION_INCREMENT;
	}
}
*/

//******************
// eMovement::DrawKnownMap
// TODO: check collision/draw layers, and draw debug rects over visited (and visible) tiles instead of entire cells.
//******************
void eMovement::DrawKnownMap() const {
	auto & tileMap = game.GetMap().TileMap();
	auto & visibleCells = game.GetMap().VisibleCells();
	for (auto & visibleCell : visibleCells) {
		if (knownMap.Index(visibleCell.first, visibleCell.second) == VISITED_TILE) {
				auto cellBounds = tileMap.Index(visibleCell.first, visibleCell.second).AbsBounds();
				game.GetRenderer().DrawIsometricRect(pinkColor, cellBounds, RENDERTYPE_DYNAMIC);
		}
	}

}