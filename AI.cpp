#include "AI.h"
#include "Game.h"

//***************
// eAI::Spawn
// TODO: make this spawn independent of eEntity::Spawn
//***************
bool eAI::Spawn() {

	if (!eEntity::Spawn())
		return false;

	collisionRadius = localBounds.Radius();
	StopMoving();
	touch.Clear();

	// knownMap dimensions, based in initialized tileMap dimensions
	knownMap.SetCellWidth(game.GetMap().TileMap().CellWidth());
	knownMap.SetCellHeight(game.GetMap().TileMap().CellHeight());
	knownMap.ClearAllCells();

	currentTile = &knownMap.Index(origin);
	previousTile = currentTile;
	lastTrailTile = nullptr;

	pathingState = COMPASS_FOLLOW_PATH;
	moveState = MOVE_TO_GOAL;
	return true;
}

//***************
// eAI::Think
// selects and updates a pathfinding type (eg: waypoint+obstacle avoid, A* optimal path, wall follow, Area awareness, raw compass?, etc)
//***************
void eAI::Think() {
	eInput * input;
	bool wasStopped;

	input = &game.GetInput();
	if (input->MousePressed(SDL_BUTTON_LEFT))
		AddUserWaypoint(eVec2(input->GetMouseX() + game.GetCamera().GetAbsBounds().x, input->GetMouseY() + game.GetCamera().GetAbsBounds().y));

	// only move with a waypoint
	if (UpdateWaypoint()) {

		wasStopped = velocity == vec2_zero;
		Move();

		UpdateKnownMap();

		if (pathingState == WALL_FOLLOW_PATH) {
			CheckCollision();	// collision detection **AND** response here
			WallFollow();
		}

		// drop a trail waypoint (but never in a deadend)
		if (moving && !wasStopped && moveState != MOVE_TO_TRAIL && lastTrailTile != currentTile) {
			trail.PushFront(origin);
			lastTrailTile = currentTile;
		}

		// check if goal is close enough to stop
		if (origin.Compare(currentWaypoint, goalRange)) {	// FIXME: wall follow starts w/o a waypoint (default)
			StopMoving();									// FIXME: this does nothing to stop a wall follower
			UpdateWaypoint(true);
		}
	}
}

//***************
// eAI::Move
//***************
void eAI::Move() {

	// two ways of settin the velocity
	if (pathingState == COMPASS_FOLLOW_PATH) {
		CompassFollow();
	} else {
		forward.vector.Set((float)((moveState == MOVE_RIGHT) - (moveState == MOVE_LEFT)), (float)((moveState == MOVE_DOWN) - (moveState == MOVE_UP)));
		velocity = forward.vector * speed;
		// FIXME: this assumes the moveState is only ONE of the four states (for an AI)
		// and pairs as-such with eEntity::CheckCollision() for collision detection AND response
		// FIXME: under what circumstances should the forward.vector or velocity be zero in wall-follow mode?
	}

	// move and prep for the next frame (wall-follow)
	if (velocity != vec2_zero) {
		moving = true;
		UpdateOrigin();
		if (pathingState == WALL_FOLLOW_PATH)
			CheckTouch();
	}
}

//***************
// eAI::WallFollow
// checks if the moveState should change
// given the current state of all ranged touch sensors
// and the current moveState
//***************
void eAI::WallFollow() {

	switch (moveState) {
		case MOVE_RIGHT: {

			// if it has lost its wall move in that direction
			// if it hasn't lost its wall move opposite that wall
			// if it never had a wall, move down
			if ((touch.oldRanged.BOTTOM_LEFT && !touch.ranged.BOTTOM_LEFT) ||
				((touch.ranged.RIGHT_TOP || touch.ranged.RIGHT_BOTTOM) && !touch.ranged.BOTTOM_LEFT))
				moveState = MOVE_DOWN;

			else if ((touch.oldRanged.TOP_LEFT && !touch.ranged.TOP_LEFT) ||
				((touch.ranged.RIGHT_TOP || touch.ranged.RIGHT_BOTTOM) && !touch.ranged.TOP_LEFT))
				moveState = MOVE_UP;

			break;
		}
		case MOVE_LEFT: {

			if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) ||
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.BOTTOM_RIGHT))
				moveState = MOVE_DOWN;

			else if ((touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT) ||
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.TOP_RIGHT))
				moveState = MOVE_UP;

			break;
		}
		case MOVE_UP: {

			if ((touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM) ||
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.RIGHT_BOTTOM))
				moveState = MOVE_RIGHT;

			else if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) ||
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.LEFT_BOTTOM))
				moveState = MOVE_LEFT;

			break;
		}
		case MOVE_DOWN: {

			if ((touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP) ||
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.RIGHT_TOP))
				moveState = MOVE_RIGHT;

			else if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) ||
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.LEFT_TOP))
				moveState = MOVE_LEFT;

			break;
		}
	}
}

//******************
// eAI::CompassFollow
// Determines the optimal movement vector to reach the current waypoint
//******************
void eAI::CompassFollow() {
	decision_t	waypoint;				// from the sprite to the next waypoint
	decision_t	test;					// vector tested for optimal travel decision
	decision_t	best;					// optimal movement
	float		distToWaypointSqr;		// to modulate speed
	float		rotationAngle;			// cumulative amount the testVector has rotated in its search
	float		maxRotation;			// to disallow vectors that backtrack if already moving
	float		weight;					// net bias for a decision about a test
	float		bestWeight;				// highest net result of all modifications to validSteps
	bool		leftOpen, rightOpen, forwardHit;

	// modulate entity speed if at least this close to a waypoint
	static const float goalRangeSqr = maxSpeed * maxSpeed * maxSteps * maxSteps;
	static const float leftBias = 1.0f;
	static const float rightBias = 1.05f;
	static const float forwardBias = 1.1f;
	static const float waypointBias = 2.0f;		// TODO: potentially modulate this during deadends (make it less strong)

	if (!moving) {
		test.vector = vec2_oneZero;
		maxRotation = 360.0f;
	} else {
		CheckWalls(leftOpen, rightOpen, forwardHit);
		test = right;		// counter-clockwise sweep of 180 degree arc from right to left in the forward direction

//		if (moveState == MOVE_TO_GOAL)
		maxRotation = 180.0f;
//		else // moveState == MOVE_TO_TRAIL
//			maxRotation = 360.0f;				// FIXME/BUG: occasionally causes semi-permanent stuck-in-place jitter

		// stuck in a corner (look for the quickest and most waypoint-oriented way out)
		if ((forward.stepRatio == 0 && right.stepRatio == 0) || (forward.stepRatio == 0 && left.stepRatio == 0))
			*currentTile = VISITED_TILE;
	}

	waypoint.vector = currentWaypoint - origin;
	if (moveState == MOVE_TO_GOAL) {							// FIXME/BUG(?): stays slow if it was slow and switched to MOVE_TO_TRAIL
		distToWaypointSqr = waypoint.vector.LengthSquared();	// TODO: potentially use this value in UpdateKnownMap();

		if (distToWaypointSqr < goalRangeSqr)
			speed = SDL_sqrtf(distToWaypointSqr) / maxSteps;	// allows more accurate approach towards goal waypoint
		else
			speed = maxSpeed;									// in the event of a near-miss
		if (speed < 1)
			speed = 1;
	}
	waypoint.vector.Normalize();

	bestWeight = 0;
	rotationAngle = 0.0f;
	while (rotationAngle < maxRotation) {

		// check how clear the path is starting one step along it
		// and head straight for the waypoint if the test.vector path crosses extremely near it
		if (CheckVectorPath(origin + (test.vector*speed), test)) {
			if (CheckVectorPath(origin + (waypoint.vector*speed), waypoint))
				forward = waypoint;
			else
				forward = test;

			// initilize the new left and right, and their validSteps that'll be used next frame
			CheckWalls(leftOpen, rightOpen, forwardHit);
			velocity = forward.vector * speed;
			return;
		}

		// FIXME/BUG: trail waypoint orbits or cannot attain sometimes 
		// POTENTIALLY fixed by putting a trail waypoint on each new tile (its never too far to navigate straight back to)

		// give the path a bias to help set priority
		weight = (float)test.validSteps;
		weight += test.vector*waypoint.vector * waypointBias;	// TODO(?): check the stepRatio along the waypointVector?
		if (moving) {
			weight += (test.vector*left.vector) * leftOpen * (leftBias * left.stepRatio);
			weight += (test.vector*right.vector) * rightOpen * (rightBias * right.stepRatio);
			weight += (test.vector*forward.vector) * !forwardHit * (forwardBias * forward.stepRatio);
		}

		// more new tiles always beats better overall weight
		if (test.stepRatio > best.stepRatio) {
			bestWeight = weight;
			best = test;
		}
		else if (test.stepRatio == best.stepRatio  && weight > bestWeight) {
			bestWeight = weight;
			best = test;
		}

		test.vector = rotationQuat_Z*test.vector; // rotate counter-clockwise
		rotationAngle += ROTATION_INCREMENT;
	}

	if (moveState == MOVE_TO_GOAL && best.stepRatio == 0) {	// deadlocked, begin deadend protocols (ie follow the trail now)
		StopMoving();
		moveState = MOVE_TO_TRAIL;
	}
	else if (moveState == MOVE_TO_TRAIL && best.stepRatio > 0) {
		StopMoving();
		moveState = MOVE_TO_GOAL;
	}
	else if (moveState == MOVE_TO_TRAIL && best.validSteps == 0) {
		StopMoving();
	}
	else {
		forward = best;
		// initilize the new left and right, and their validSteps that'll be used next frame
		CheckWalls(leftOpen, rightOpen, forwardHit);
	}
	// moveState may have changed, track the correct waypoint
	UpdateWaypoint();
	velocity = forward.vector * speed;
}

//******************
// eAI::CheckVectorPath
// determines the state of the entity's position for the next few frames
// return true if a future position using along is near the waypoint
//******************
bool eAI::CheckVectorPath(eVec2 from, decision_t & along) {
	eVec2 testPoint;
	int newSteps;

	along.validSteps = 0;
	along.stepRatio = 0.0f;
	newSteps = 0;
	while (along.validSteps < maxSteps) {

		// forward test point (starts on circle circumscribing the sprite bounding box)
		testPoint.x = from.x + (collisionRadius*along.vector.x);
		testPoint.y = from.y + (collisionRadius*along.vector.y);

		// check for collision
		if (!(game.GetMap().IsValid(testPoint)))
			break;

		// forward test point rotated clockwise 90 degrees
		testPoint.x = from.x + (collisionRadius*along.vector.y);
		testPoint.y = from.y - (collisionRadius*along.vector.x);

		// check for collision
		if (!(game.GetMap().IsValid(testPoint)))
			break;

		// forward test point rotated counter-clockwise 90 degrees
		testPoint.x = from.x - (collisionRadius*along.vector.y);
		testPoint.y = from.y + (collisionRadius*along.vector.x);

		// check for collision
		if (!(game.GetMap().IsValid(testPoint)))
			break;

		// all test points validated
		along.validSteps++;

		// check if the step falls on an unexplored tile
		if (knownMap.Index(from) == UNKNOWN_TILE)
			newSteps++;

		// check if the goal waypoint is near the center of the validated test position
		if (moveState == MOVE_TO_GOAL && from.Compare(currentWaypoint, goalRange))
			return true;

		// move to check validity of next position
		from += along.vector*speed;
	}

	if (along.validSteps == 0)
		along.stepRatio = 0;
	else
		along.stepRatio = (float)newSteps / (float)along.validSteps;

	return false;
}

//**************
// eAI::CheckWalls
// assigns the vectors perpendicular to the forward vector
// and checks if the range along them has significantly changed
//**************
void eAI::CheckWalls(bool & leftOpen, bool & rightOpen, bool & forwardHit) {
	int oldLeftSteps = left.validSteps;
	int oldRightSteps = right.validSteps;
	static const int stepIncreaseThreshold = 2;

	left.vector.Set(-forward.vector.y, forward.vector.x);	// forward rotated 90 degrees counter-clockwise
	right.vector.Set(forward.vector.y, -forward.vector.x);	// forward rotated 90 degrees clockwise

	CheckVectorPath(origin + (forward.vector*speed), forward);
	CheckVectorPath(origin + (left.vector*speed), left);
	CheckVectorPath(origin + (right.vector*speed), right);

	forwardHit = forward.validSteps == 0;
	leftOpen = left.validSteps >= oldLeftSteps + stepIncreaseThreshold;
	rightOpen = right.validSteps >= oldRightSteps + stepIncreaseThreshold;
}


//******************
// eAI::CheckFogOfWar
// TODO: have the map object check if ANY of the team entities have visited a tile about to be drawn, if not draw black,
// if so, then the map goes through all entities calling CheckFogOfWar, if ONE returns true then it'll stop sweep and draw bright,
// if none return true by the end of the sweep, then draw dim ( reduce sweep time by using a locational Potential_Visible_Set )
//******************
bool eAI::CheckFogOfWar(const eVec2 & point) const {
	eVec2 lineOfSight;

	lineOfSight = point - origin;
	return lineOfSight.LengthSquared() <= sightRange;
}

//******************
// eAI::CheckTouch
// Sets a sensor bit for every point within the entity's range in a non-traversable area
// --the entity effectively has 16 touch sensors--
//******************
void eAI::CheckTouch() {

	// on-sprite checks
	touch.oldLocal = touch.local;
	touch.local.Clear();

	// FIXME: (size was 15) bounds is 16 wide and high, the -1 -2 situation seems arbitrary,
	// and also fails if the bounds is rotated
	// horizontally oriented sensors
	touch.local.RIGHT_TOP		= !game.GetMap().IsValid(eVec2(absBounds[1].x - 1, absBounds[0].y + 1));
	touch.local.RIGHT_BOTTOM	= !game.GetMap().IsValid(eVec2(absBounds[1].x - 1, absBounds[1].y - 2));
	touch.local.LEFT_BOTTOM		= !game.GetMap().IsValid(eVec2(absBounds[0].x, absBounds[1].y - 2));
	touch.local.LEFT_TOP		= !game.GetMap().IsValid(eVec2(absBounds[0].x, absBounds[0].y + 1));

	// vertically oriented sensors
	touch.local.TOP_LEFT		= !game.GetMap().IsValid(eVec2(absBounds[0].x + 1, absBounds[0].y));
	touch.local.TOP_RIGHT		= !game.GetMap().IsValid(eVec2(absBounds[1].x - 2, absBounds[0].y));
	touch.local.BOTTOM_RIGHT	= !game.GetMap().IsValid(eVec2(absBounds[1].x - 2, absBounds[1].y - 1));
	touch.local.BOTTOM_LEFT		= !game.GetMap().IsValid(eVec2(absBounds[0].x + 1, absBounds[1].y - 1));

	// ranged off-sprite checks
	touch.oldRanged = touch.ranged;
	touch.ranged.Clear();

	// horizontally oriented sensors
	touch.ranged.RIGHT_TOP		= !game.GetMap().IsValid(eVec2(absBounds[1].x + touch.reach - 1, absBounds[0].y));
	touch.ranged.RIGHT_BOTTOM	= !game.GetMap().IsValid(eVec2(absBounds[1].x + touch.reach - 1, absBounds[1].y - 1));
	touch.ranged.LEFT_TOP		= !game.GetMap().IsValid(eVec2(absBounds[0].x - touch.reach, absBounds[0].y));
	touch.ranged.LEFT_BOTTOM	= !game.GetMap().IsValid(eVec2(absBounds[0].x - touch.reach, absBounds[1].y - 1));

	// vertically oriented sensors
	touch.ranged.TOP_RIGHT		= !game.GetMap().IsValid(eVec2(absBounds[1].x - 1, absBounds[0].y - touch.reach));
	touch.ranged.TOP_LEFT		= !game.GetMap().IsValid(eVec2(absBounds[0].x, absBounds[0].y - touch.reach));
	touch.ranged.BOTTOM_RIGHT	= !game.GetMap().IsValid(eVec2(absBounds[1].x - 1, absBounds[1].y + touch.reach - 1));
	touch.ranged.BOTTOM_LEFT	= !game.GetMap().IsValid(eVec2(absBounds[0].x, absBounds[1].y + touch.reach - 1));
}

//******************
// eAI::CheckCollision
// FIXME: this should belong to a Collision class that checks for overlapping bounds via graph nodes
// then performs the collision response accordingly
// Isolated check for overlap into non-traversable areas, and immediate sprite position correction
//******************
void eAI::CheckCollision() {
	eBounds tileAbsBounds;	// tile the entity should be on the edge of in the event of collision
	eVec2 correction;
	int row, column;

	static const byte_t * knownMapStart = &knownMap.Index(0, 0);

	// straight-on wall collision
	// correction adjusts the leading edge of collision box
	if (touch.local != touch.oldLocal) {
		correction = vec2_zero;
		knownMap.Index(oldOrigin, row, column);
		tileAbsBounds = eBounds(eVec2((float)(row * knownMap.CellWidth()), (float)(column * knownMap.CellHeight())),
			eVec2((float)(row * knownMap.CellWidth() + knownMap.CellWidth()), (float)(column * knownMap.CellHeight() + knownMap.CellHeight())));

		switch (moveState) {
			case MOVE_RIGHT: {
				if (touch.local.RIGHT_TOP || touch.local.RIGHT_BOTTOM)
					correction.x = tileAbsBounds[1].x - absBounds[1].x;			// right edge of currentTile
				break;
			}
			case MOVE_LEFT: {
				if (touch.local.LEFT_TOP || touch.local.LEFT_BOTTOM)
					correction.x = tileAbsBounds[0].x - absBounds[0].x;			// left edge of currentTile
				break;
			}
			case MOVE_UP: {
				if (touch.local.TOP_RIGHT || touch.local.TOP_LEFT)
					correction.y = tileAbsBounds[0].y - absBounds[0].y;			// top edge of currentTile
				break;
			}
			case MOVE_DOWN: {
				if (touch.local.BOTTOM_RIGHT || touch.local.BOTTOM_LEFT)
					correction.y = tileAbsBounds[1].y - absBounds[1].y;			// bottom edge of currentTile
				break;
			}
		}
		SetOrigin(origin + correction);
		// used to return here if a correction occured
	}

	// wall-follower AI outside turn wall alignment
	// correction adjusts the trailing edge of collision box
	if (touch.ranged != touch.oldRanged) {
		correction = vec2_zero;
		knownMap.Index(previousTile, row, column);
		tileAbsBounds = eBounds(eVec2((float)(row * knownMap.CellWidth()), (float)(column * knownMap.CellHeight())),
			eVec2((float)(row * knownMap.CellWidth() + knownMap.CellWidth()), (float)(column * knownMap.CellHeight() + knownMap.CellHeight())));

		switch (moveState) {
			case MOVE_RIGHT: {
				if ((touch.oldRanged.BOTTOM_LEFT && !touch.ranged.BOTTOM_LEFT) ||
					(touch.oldRanged.TOP_LEFT && !touch.ranged.TOP_LEFT))
					correction.x = tileAbsBounds[1].x - absBounds[0].x;			// right edge of previousTile
				break;
			}
			case MOVE_LEFT: {
				if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) ||
					(touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT))
					correction.x = tileAbsBounds[0].x - absBounds[1].x;			// left edge of previousTile
				break;
			}
			case MOVE_UP: {
				if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) ||
					(touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM))
					correction.y = tileAbsBounds[0].y - absBounds[1].y;			// top edge of previousTile
				break;
			}
			case MOVE_DOWN: {
				if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) ||
					(touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP))
					correction.y = tileAbsBounds[1].y - absBounds[0].y;			// bottom edge of previousTile			
				break;
			}
		}
		SetOrigin(origin + correction);
	}
}

//******************
// eAI::AddUserWaypoint
// TODO: use SpatialIndexGrid::Validate(point) to snap a waypoint onto the nearest
// in-bounds map point (like Age of Empires flags)
//******************
void eAI::AddUserWaypoint(eVec2 & waypoint) {

	// TODO: disallow waypoints on collision tiles and off-map
	// allow other entities to become waypoints (that move)
	// moving waypoints would need to have their info updated in the deque (hence pointers instead of copies)

	if (game.GetMap().IsValid(waypoint)) {
		goals.PushFront(waypoint);
		UpdateWaypoint();
	}
}

//******************
// eAI::UpdateWaypoint
// returns false if there's no waypoints available
// TODO: make currentWaypoint a pointer, then set it to nullptr if there's no waypoints, then make this void return-type
//******************
bool eAI::UpdateWaypoint(bool getNext) {
	switch (moveState) {
	case MOVE_TO_GOAL: {
		if (getNext) {
			goals.PopBack();
			trail.Clear();
		}
		CheckTrail();
		if (!goals.IsEmpty()) {
			currentWaypoint = goals.Back();
			return true;
		}
		return false;
	}
	case MOVE_TO_TRAIL: {
		if (getNext)
			trail.PopFront();
		if (!CheckTrail()) {
			currentWaypoint = trail.Front();
			return true;
		}
		else if (!goals.IsEmpty()) {
			moveState = MOVE_TO_GOAL;
			currentWaypoint = goals.Back();
			return true;
		}
		return false;
	}
	default:
		return pathingState == WALL_FOLLOW_PATH;
	}
}

//******************
// eAI::CheckTrail
// determine if the entity should fresh-start goal pathfinding
//******************
bool eAI::CheckTrail() {
	if (trail.IsEmpty()) {
		knownMap.ClearAllCells();
		lastTrailTile = nullptr;
		return true;
	}
	return false;
}

//******************
// eAI::UpdateKnownMap
// marks the currentTile as VISITED_TILE, clears out un-needed trail waypoints,
// and resets tiles around the current goal waypoint to UNKNOWN_TILE
//******************
void eAI::UpdateKnownMap() {
	byte_t * checkTile;
	int row, column;
	int startRow, startCol;
	int endRow, endCol;
	int tileResetRange;		// size of the box around the goal to set tiles to UNKNOWN_TILE

	// mark the tile to help future movement decisions
	// FIXME/BUG: make this/add a collidedTile, then collision correct (to fix bug where wall-follower marking too many tiles)
	checkTile = &knownMap.Index(origin);
	if (checkTile != currentTile) {
		previousTile = currentTile;
		*previousTile = VISITED_TILE;
		currentTile = checkTile;
	}

	tileResetRange = 0;
	if (!CheckKnownMapCleared()) {
		
		// solid-box of tiles at the tileResetRange centered on **the current goal waypoint** to to reset the knownMap
		// FIXME: this Length() call is very costly and shouldn't run each frame
		// SOLUTION: find a workaround for using the knownMap at all, and/or prevent the current "orbit" behavior
		// that the AI displays when attempting to attain a goal (ie sometimes there's an opening that's missed in favor
		// of maintaining the directional weight)
		if (!goals.IsEmpty()) {
			tileResetRange = (int)((goals.Back() - origin).Length() / (knownMap.CellWidth() * 2));

			// find the knownMap row and column of the current goal waypoint
			knownMap.Index(goals.Back(), row, column);

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
		if (knownMap.Index(trail.Back()) == UNKNOWN_TILE)
			trail.PopBack();
		else
			break;
	}
}

//***************
// eAI::Draw
// TODO: make this draw independent of eEntity::Draw()
//***************
void eAI::Draw() {
	DrawKnownMap();	
	DrawTrailWaypoints();
	DrawGoalWaypoints();
	eEntity::Draw();
	DrawCollisionCircle();
	DrawTouchSensors();
}

//******************
// eAI::DrawGoalWaypoints
//******************
void eAI::DrawGoalWaypoints() const {
	static eImage * const debugImage = sprite.Image();
	eVec2 debugPoint;
	int node;

	if (!game.debugFlags.GOAL_WAYPOINTS)
		return;

	node = 0;
	while (node < goals.Size()) {
		debugPoint = goals.FromBack(node) - game.GetCamera().GetAbsBounds();
		debugPoint.SnapInt();
		game.GetRenderer().DrawImage(debugImage, (eBounds(debugPoint).ExpandSelf(8))[0]);	// top-left corner
		node++;

	}
}

//******************
// eAI::DrawTrailWaypoints
//******************
void eAI::DrawTrailWaypoints() const {
	static eImage * const debugImage = sprite.Image();
	eVec2 debugPoint;
	int node;

	if (!game.debugFlags.TRAIL_WAYPOINTS)
		return;

	node = 0;
	while (node < trail.Size()) {
		debugPoint = trail.FromFront(node) - game.GetCamera().GetAbsBounds();
		debugPoint.SnapInt();
		game.GetRenderer().DrawImage(debugImage, (eBounds(debugPoint).ExpandSelf(8))[0]);	// top-left corner
		node++;
	}
}

//******************
// eAI::DrawCollisionCircle
//******************
void eAI::DrawCollisionCircle() const {
	eVec2 debugVector;
	eVec2 debugPoint;
	float rotationAngle;
	int pink[3] = { 255, 0, 255 };
	int blue[3] = { 0,0,255 };
	int * color;

	if (!game.debugFlags.COLLISION_CIRCLE)
		return;

	// draws one pixel for each point on the current collision circle 
	if (moveState == MOVE_TO_GOAL)
		color = pink;
	else
		color = blue;

	debugVector = vec2_oneZero;
	rotationAngle = 0.0f;
	while (rotationAngle < 360.0f) {

		if (velocity*debugVector >= 0) {
			debugPoint = origin + (debugVector * collisionRadius) - game.GetCamera().GetAbsBounds();
			debugPoint.SnapInt();
			game.GetRenderer().DrawPixel(debugPoint, color[0], color[1], color[2]);
		}
		debugVector = rotationQuat_Z*debugVector;	// rotate counter-clockwise
		rotationAngle += ROTATION_INCREMENT;
	}
}

//******************
// eAI::DrawTouchSensors
// debug screen printout of 8 ranged entity touch sensors' statuses
// TODO(?): make a toggleable console, then print text to that
//******************
void eAI::DrawTouchSensors() const {
	char buffer[64];

	if (!game.debugFlags.TOUCH_SENSORS)
		return;

	// touch sensors
	sprintf_s(buffer, "%s|%s", touch.ranged.TOP_LEFT ? "TL" : " ",
		touch.ranged.TOP_RIGHT ? "TR" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(100.0f, 150.0f), 255, 0, 0);

	sprintf_s(buffer, "%s|%s", touch.ranged.RIGHT_TOP ? "RT" : " ",
		touch.ranged.RIGHT_BOTTOM ? "RB" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(200.0f, 200.0f), 0, 255, 0);

	sprintf_s(buffer, "%s|%s", touch.ranged.BOTTOM_RIGHT ? "BR" : " ",
		touch.ranged.BOTTOM_LEFT ? "BL" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(150.0f, 250.0f), 0, 0, 255);

	sprintf_s(buffer, "%s|%s", touch.ranged.LEFT_BOTTOM ? "LB" : " ",
		touch.ranged.LEFT_TOP ? "LT" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(50.0f, 200.0f), 255, 255, 0);

	// FIXME: account for diagonal motion on printout
	// movement direction
	sprintf_s(buffer, "%s", moveState == MOVE_UP ? "UP" : moveState == MOVE_DOWN ? "DOWN" : moveState == MOVE_RIGHT ? "RIGHT" : moveState == MOVE_LEFT ? "LEFT" : moveState == MOVE_TO_GOAL ? "GOAL" : moveState == MOVE_TO_TRAIL ? "TRIAL" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(150.0f, 300.0f), 255, 255, 0);

	// entity position (center of bounding box)
	sprintf_s(buffer, "%i, %i", (int)Origin().x, (int)Origin().y);
	game.GetRenderer().DrawOutlineText(buffer, eVec2(150.0f, 350.0f), 255, 255, 255);

}

//******************
// eAI::DrawKnownMap
//******************
void eAI::DrawKnownMap() const {
	SDL_Rect screenRect;
	eVec2 screenPoint;
	int row, column;
	int endRow, startCol, endCol;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond in all directions)
	// TODO: allow this value to change in the event that cell size changes
	static const int maxScreenRows = (int)(game.GetRenderer().Width() / knownMap.CellWidth()) + 2;
	static const int maxScreenColumns = (int)(game.GetRenderer().Height() / knownMap.CellHeight()) + 2;
	
	if (!game.debugFlags.KNOWN_MAP_DRAW)
		return;

	// initialize the area of the tileMap to query
	knownMap.Index(game.GetCamera().GetAbsBounds(), row, column);
	startCol = column;
	endRow = row + maxScreenRows;
	endCol = column + maxScreenColumns;
	knownMap.Validate(endRow, endCol);

	screenRect.w = knownMap.CellWidth();
	screenRect.h = knownMap.CellHeight();
	while (row <= endRow) {
		if (knownMap.Index(row, column) == VISITED_TILE) {
			screenRect.x = eMath::NearestInt(((float)(row * knownMap.CellWidth()) - game.GetCamera().GetAbsBounds().x));
			screenRect.y = eMath::NearestInt(((float)(column * knownMap.CellHeight()) - game.GetCamera().GetAbsBounds().y));
			game.GetRenderer().DrawClearRect(screenRect);
		}
		
		column++;
		if (column > endCol) {
			column = startCol;
			row++;
		}
	}
}

//******************
// eAI::CheckClearKnownmap
//******************
bool eAI::CheckKnownMapCleared() {
	eInput * input;

	input = &game.GetInput();
	if (game.debugFlags.KNOWN_MAP_CLEAR && input->KeyPressed(SDL_SCANCODE_R)) {
		knownMap.ClearAllCells();
		return true;
	}

	return false;
}