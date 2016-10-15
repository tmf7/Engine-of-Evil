#include "Game.h"

// FIXME: these variable names are too verbose and not specific enough
Entity::Entity() {

	speed = MAX_SPEED;
	size = 15;
	sightRange = 128;
	sight.w = sightRange;
	sight.h = sightRange;
	touchRange = 1;
	waypointRange = speed;

	rotationQuat_Z.Set(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));
	moveState = MOVE_TO_GOAL;	// MOVE_RIGHT;
	oldTouch = 0;
	touch = 0;

	frameDelayCount = 0;

	StopMoving();
}

// TODO: insert use of the sourceRect for initial frame of animation
bool Entity::Init(char fileName[], bool key, Game * const game) {

	SDL_Surface* surface = NULL;

	if (!game || !game->GetMap())
		return false;

	this->game = game;

	if (!fileName[0])
		return false;

	surface = SDL_LoadBMP(fileName);

	if (!surface)
		return false;

	sprite = SDL_ConvertSurface(surface, game->GetBuffer()->format, 0);

	SDL_FreeSurface(surface);
	surface = NULL;

	if (sprite && key) {

		Uint32 colorKey = SDL_MapRGB(sprite->format, 255, 0, 255);
		SDL_SetColorKey(sprite, SDL_TRUE, colorKey);
	}

	collisionRadius = (float)((sprite->w * sprite->w) + (sprite->h * sprite->h));
	collisionRadius = SDL_sqrtf(collisionRadius);
	collisionRadius /= 2.0f;

	// FIXME(?): this relies on the map being initialized already
	// knownMap dimensions
	knownMapRows = game->GetMap()->GetRows();
	knownMapCols = game->GetMap()->GetColumns();
	memset(&knownMap[0][0], UNKNOWN_TILE, MAX_MAP_ROWS * MAX_MAP_COLUMNS * sizeof(knownMap[0][0]));	// ensure all used tiles initilized to 0

	Spawn();

	return true;
}

void Entity::Free() {

	if (sprite) {
		SDL_FreeSurface(sprite);
		sprite = NULL;
	}
}

// FIXME(?): GetMapIndex(row,column) can return -1 if the row or column is out of bounds
// FIXME: if radius exceeds the map's size in either direction, generate a new map (or local area)
// TODO: determine a failure to spawn condition (ie when to return false)
// TODO: create different enums for AI placement, including specific positions
// Place the entity on the top-leftmost walkable tile (in an expanding block "radius")
void Entity::Spawn()
{
	int i, j;
	int radius = 1;
	int tileSize = game->GetMap()->GetTileSize();
	bool entity_placed = false;

	// Check the top-left corner first
	if (game->GetMap()->IndexValue(0,0) == TRAVERSABLE_TILE) {

		spritePos.x = 5;
		spritePos.y = 5;
		UpdateCenter();
		currentTile = &knownMap[0][0];
		lastTrailTile = nullptr;
		return;
	}

	// Expand the search
	while (!entity_placed) {

		// down from the top
		for (i = radius, j = 0; j <= radius; j++) {

			if (game->GetMap()->IndexValue(i, j) == TRAVERSABLE_TILE) {
				spritePos.x = (float)(i*tileSize + 5);
				spritePos.y = (float)(j*tileSize + 5);
				entity_placed = true;
				break;
			}
		}

		// TODO: test to ensure this loop isnt entered if (entity_placed)
		// in from the bottom of the last search
		for (i = radius - 1, j = radius; i >= 0 && !entity_placed; i--) {

			if (game->GetMap()->IndexValue(i, j) == TRAVERSABLE_TILE) {

				spritePos.x = (float)(i*tileSize + 5);
				spritePos.y = (float)(j*tileSize + 5);
				entity_placed = true;
				break;
			}
		}

		radius++;
	}
	UpdateCenter();
	currentTile = &knownMap[i][j];
	lastTrailTile = nullptr;
}

// TODO: add animation frame selection here
void Entity::Update() {

	SDL_Rect destRect;
	eVec2 debugVector = ORIGIN_VEC2;
	eVec2 debugWaypoint;
	float rotationAngle;
	int collisionX, collisionY;
	int node;
	int pink[3] = { 255, 0, 255 };
	int blue[3] = { 0,0,255 };
	int * color;
//	int columns = image->w / width;

// BEGIN FREEHILL DEBUG knownMap memset test
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_R])
		memset(&knownMap[0][0], UNKNOWN_TILE, MAX_MAP_ROWS * MAX_MAP_COLUMNS * sizeof(knownMap[0][0]));
// END FREEHIL DEBUG knownMap memset test

	Move();

/*
	if (frameDelayCount > frameDelay){

		frameDelayCount = 0;
		currentFrame++;
	}

	if (currentFrame > lastFrame)
		currentFrame = firstFrame;

	SDL_Rect sourceRect;
	sourceRect.y = 0;			// (frame / columns)*height;
	sourceRect.x = (frame%columns)*width;
	sourceRect.w = width;
	sourceRect.h = height;

	SDL_BlitSurface(sprite, &sourceRect, game->GetBuffer, &destRect);
*/
	// draw all waypoints
	// FIXME: draws regardless if they're visible
	node = 0;
	while(node < goals.Size()) {
		debugWaypoint = goals.FromBack(node);

		destRect.x = (int)(debugWaypoint.x) - game->GetMap()->GetCamera().x;
		destRect.y = (int)(debugWaypoint.y) - game->GetMap()->GetCamera().y;
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
		node++;
		
	}

	node = 0;
	while (node < trail.Size()) {
		debugWaypoint = trail.FromFront(node);

		destRect.x = (int)(debugWaypoint.x) - game->GetMap()->GetCamera().x;
		destRect.y = (int)(debugWaypoint.y) - game->GetMap()->GetCamera().y;
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
		node++;
	}

	// draw sprite 
	// FIXME: draws regardless if it's visible
	destRect.x = (int)(spritePos.x) - game->GetMap()->GetCamera().x;
	destRect.y = (int)(spritePos.y) - game->GetMap()->GetCamera().y;
	SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);

	// FREEHILL BEGIN DEBUG COLLISION CIRCLE

	// draws one pink pixel for each unique x,y point on the current collision circle 
	// FIXME: draws regardless if they're visible
	if (moveState == MOVE_TO_GOAL)
		color = pink;
	else
		color = blue;

	rotationAngle = 0.0f;
	while (rotationAngle < 360.0f) {

		if (forward.vector*debugVector >= 0) {

			collisionX = spriteCenter.x + (int)(collisionRadius*debugVector.x) - game->GetMap()->GetCamera().x;
			collisionY = spriteCenter.y + (int)(collisionRadius*debugVector.y) - game->GetMap()->GetCamera().y;
			DrawPixel(game->GetBuffer(), collisionX, collisionY, color[0], color[1], color[2]);
		}
		debugVector = rotationQuat_Z*debugVector;	// rotate counter-clockwise
		rotationAngle += ROTATION_INCREMENT;
	}
	// FREEHILL END DEBUG COLLISION CIRCLE
}

// Ultimate Goal: select a pathfinding type (eg: compass, endpoint+obstacle adjust, waypoints+minipaths, wall follow, Area awareness, etc)
// Gather sensor information and decide how to move
void Entity::Move() {
	int * checkTile;

	//CheckFogOfWar();	// also uncomment the Update() code in Map.cpp

	////////////////////////////////////////////
	//BEGIN WAYPOINT VECTOR MOVEMENT ALGORITHM//
	////////////////////////////////////////////

	// don't move without a waypoint
	if (!UpdateWaypoint())
		return;

	UpdateMovement();
	UpdatePosition();

	if ( spriteCenter.Compare(currentWaypoint, waypointRange) ) {
		StopMoving();
		UpdateWaypoint(true);
	}

	// mark the tile to help future movement decision
	checkTile = KnownMapIndex(spriteCenter);
	if ( checkTile != nullptr && checkTile != currentTile ) {
		UpdateKnownMap();
		currentTile = checkTile;
	}


	//////////////////////////////////////////
	//END WAYPOINT VECTOR MOVEMENT ALGORITHM//
	//////////////////////////////////////////

	/* 
	////////////////////////////////////////
	//BEGIN WALL FOLLOW MOVEMENT ALGORITHM//
	////////////////////////////////////////

	oldMoveState = moveState;

	x -= speed * ((moveState & MOVE_LEFT) > 0);
	x += speed * ((moveState & MOVE_RIGHT) > 0);

	CheckCollision(true, false);

	switch (moveState) {

		case MOVE_RIGHT: {

			// set additional enemy awareness
			watch_touch = (RIGHT_TOP | RIGHT_BOTTOM);

			// if it has lost its wall move in that direction
			// if it hasn't lost its wall move opposite that wall
			// if it never had a wall, move down
			if ((oldTouch & ~touch & BOTTOM_LEFT) || ((touch & watch_touch) && (~touch & BOTTOM_LEFT)))
				moveState = MOVE_DOWN;

			else if ((oldTouch & ~touch & TOP_LEFT) || ((touch & watch_touch) && (~touch & TOP_LEFT)))
				moveState = MOVE_UP;

			if (moveState != oldMoveState)
				return;

			break;
		}

		// rinse, repeat
		case MOVE_LEFT: {

			watch_touch = (LEFT_TOP | LEFT_BOTTOM);

			if ((oldTouch & ~touch & BOTTOM_RIGHT) || ((touch & watch_touch) && (~touch & BOTTOM_RIGHT)))
				moveState = MOVE_DOWN;

			else if ((oldTouch & ~touch & TOP_RIGHT) || ((touch & watch_touch) && (~touch & TOP_RIGHT)))
				moveState = MOVE_UP;

			if (moveState != oldMoveState)
				return;

			break;
		}
	}

	y -= speed * ((moveState & MOVE_UP) > 0);
	y += speed * ((moveState & MOVE_DOWN) > 0);

	CheckCollision(false, true);

	switch (moveState) {

		case MOVE_UP: {

			watch_touch = (TOP_LEFT | TOP_RIGHT);

			if ((oldTouch & ~touch & RIGHT_BOTTOM) || ((touch & watch_touch) && (~touch & RIGHT_BOTTOM)))
				moveState = MOVE_RIGHT;

			else if ((oldTouch & ~touch & LEFT_BOTTOM) || ((touch & watch_touch) && (~touch & LEFT_BOTTOM)))
				moveState = MOVE_LEFT;

			if (moveState != oldMoveState)
				return;

			break;
		}

		case MOVE_DOWN: {

			watch_touch = (BOTTOM_LEFT | BOTTOM_RIGHT);

			if ((oldTouch & ~touch & RIGHT_TOP) || ((touch & watch_touch) && (~touch & RIGHT_TOP)))
				moveState = MOVE_RIGHT;

			else if ((oldTouch & ~touch & LEFT_TOP) || ((touch & watch_touch) && (~touch & LEFT_TOP)))
				moveState = MOVE_LEFT;

			if (moveState != oldMoveState)
				return;

			break;
		}
	}

	//////////////////////////////////////
	//END WALL FOLLOW MOVEMENT ALGORITHM//
	//////////////////////////////////////
	*/
}

//******************
// UpdateMovement
// Determines the optimal movement vector to reach the current waypoint
//******************
void Entity::UpdateMovement() {
	eVec2 oldForward;					// previously used forward.vector
	decision_t waypoint;				// from the sprite to the next waypoint
	decision_t test;					// vector tested for optimal travel decision
	decision_t best;					// optimal movement
	float distToWaypointSqr;			// to modulate speed
	float rotationAngle;				// cumulative amount the testVector has rotated in its search
	float maxRotation;					// to disallow vectors that backtrack if already moving
	float weight;						// net bias for a decision about a test
	float bestWeight;					// highest net result of all modifications to validSteps
	int walls;							// determines the bias that will be given to each test
	int * resetTile;					// used for overwritten trail waypoints

	// modulate entity speed if at least this close to a waypoint
	static const int goalRangeSqr = MAX_SPEED * MAX_SPEED * MAX_STEPS * MAX_STEPS;

	walls = 0;
	if (!moving) {
		test.vector = ORIGIN_VEC2;
		maxRotation = 360.0f;
	} else {
		CheckWalls(walls);		// sets left and right values
		test = right;			// counter-clockwise sweep of 180 degree arc from right to left in the forward direction

//		if (moveState == MOVE_TO_GOAL)
			maxRotation = 180.0f;
//		else // moveState == MOVE_TO_TRAIL
//			maxRotation = 360.0f;				// FIXME/BUG: occasionally causes semi-permanent stuck-in-place jitter

		// stuck in a corner (look for the quickest and most waypoint-oriented way out)
		if ((forward.stepRatio == 0 && right.stepRatio == 0) || (forward.stepRatio == 0 && left.stepRatio == 0))
			UpdateKnownMap();
	}

	waypoint.vector = currentWaypoint - spriteCenter;
	if (moveState == MOVE_TO_GOAL) {
		distToWaypointSqr = waypoint.vector.LengthSquared();

		if (distToWaypointSqr < goalRangeSqr)
			speed = (int)(SDL_sqrtf(distToWaypointSqr) / MAX_STEPS);	// allows more accurate approach towards goal waypoint
		else
			speed = MAX_SPEED;											// in the event of a near-miss
		if (speed < 1)
			speed = 1;
	}
	waypoint.vector.Normalize();

	bestWeight = 0;
	rotationAngle = 0.0f;
	oldForward = forward.vector;
	while (rotationAngle < maxRotation) {

		// check how clear the path is starting one step along it
		// and head straight for the waypoint if the test.vector path crosses extremely near it
		if ( CheckMovement( spriteCenter+(test.vector*speed), test ) ) {
			if ( CheckMovement( spriteCenter+(waypoint.vector*speed), waypoint) )
				forward = waypoint;
			else
				forward = test;

			CheckWalls(walls);	// to update the validSteps that the sprite will be comparing on the next frame
			moving = true;
			return;
		}

		// FIXME/BUG: trail waypoint orbits or cannot attain sometimes 
		// POTENTIALLY fixed by putting a trail waypoint on each new tile (its never too far to navigate straight back to)

		// FIXME/BUG: using stepRatio as the abc_BIAS modifier [0,1] may be the cause of the entity following
		// "corners" created by its VISITED_TILES and non-sensibly follow corners that lead away from an obviously better path
		// give the path a bias to help set priority
		weight = test.validSteps;
		weight += test.vector*waypoint.vector * WAYPOINT_BIAS;	// TODO(?): check the stepRatio along the waypointVector?
		if (moving) {	
			weight += (test.vector*left.vector) * ((walls&LEFT_WALL_OPENED) > 0) * (LEFT_BIAS * left.stepRatio);
			weight += (test.vector*right.vector) * ((walls&RIGHT_WALL_OPENED) > 0) * (RIGHT_BIAS * right.stepRatio);
			weight += (test.vector*forward.vector) * !((walls&FORWARD_WALL_HIT) > 0) * (FORWARD_BIAS * forward.stepRatio);
		}

		// more new tiles always beats better overall weight
		if (test.stepRatio > best.stepRatio) {
			bestWeight = weight;
			best = test;
		} else if (test.stepRatio == best.stepRatio  && weight > bestWeight) {
			bestWeight = weight;
			best = test;
		}

		test.vector = rotationQuat_Z*test.vector; // rotate counter-clockwise
		rotationAngle += ROTATION_INCREMENT;
	}

	if (moveState == MOVE_TO_GOAL && best.stepRatio == 0) {	// deadlocked, begin deadend protocols (ie follow the trail now)
		StopMoving();
		moveState = MOVE_TO_TRAIL;
	} else if (moveState == MOVE_TO_TRAIL && best.stepRatio > 0) {
		StopMoving();
		moveState = MOVE_TO_GOAL;
	} else if (moveState == MOVE_TO_TRAIL && best.validSteps == 0) {	
		StopMoving();
	} else {
		forward = best;
		CheckWalls(walls);	// to update the validSteps that the sprite will be comparing on the next frame
		moving = true;		
	}
	UpdateWaypoint();

	// drop a trail waypoint and reset to UNKNOWN_TILE any overwritten trail waypoints
	if ( moving && moveState == MOVE_TO_GOAL &&  oldForward != ZERO_VEC2 &&  lastTrailTile != currentTile ) {
		trail.PushFront(spriteCenter);
		lastTrailTile = currentTile;
	}
}

//******************
// CheckMovement
// determines the state of the sprite's position for the next few frames
// return true if a future position using along is near the waypoint
//******************
bool Entity::CheckMovement(eVec2 from, decision_t & along) {
	eVec2 testPoint;
	int newSteps;

	along.validSteps = 0;
	along.stepRatio = 0.0f;
	newSteps = 0;
	while (along.validSteps < MAX_STEPS) {

		// forward test point (starts on circle circumscribing the sprite bounding box)
		testPoint.x = from.x + (collisionRadius*along.vector.x);
		testPoint.y = from.y + (collisionRadius*along.vector.y);

		// check for collision
		if (!(game->GetMap()->IsValid(testPoint)))
			break;

		// forward test point rotated clockwise 90 degrees
		testPoint.x = from.x + (collisionRadius*along.vector.y);
		testPoint.y = from.y - (collisionRadius*along.vector.x);

		// check for collision
		if (!(game->GetMap()->IsValid(testPoint)))
			break;

		// forward test point rotated counter-clockwise 90 degrees
		testPoint.x = from.x - (collisionRadius*along.vector.y);
		testPoint.y = from.y + (collisionRadius*along.vector.x);

		// check for collision
		if (!(game->GetMap()->IsValid(testPoint)))
			break;

		// all test points validated
		along.validSteps++;

		// check if the step falls on an unexplored tile
		if (KnownMapValue(from) == UNKNOWN_TILE)
			newSteps++;

		// check if the goal waypoint is near the center of the validated test position
		if (moveState == MOVE_TO_GOAL && from.Compare(currentWaypoint, waypointRange))
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
// CheckWalls
// assigns the vectors perpendicular to the forward vector
// and checks if the range along them has significantly changed
//**************
void Entity::CheckWalls(int & walls) {
	int oldLeftSteps = left.validSteps;
	int oldRightSteps = right.validSteps;

	right.vector.Set(forward.vector.y, -forward.vector.x);	// forward rotated 90 degrees clockwise
	left.vector.Set(-forward.vector.y, forward.vector.x);	// forward rotated 90 degrees counter-clockwise

	CheckMovement(spriteCenter + (forward.vector*speed), forward);
	CheckMovement(spriteCenter + (left.vector*speed), left);
	CheckMovement(spriteCenter + (right.vector*speed), right);

	walls = 0;
	if (forward.validSteps == 0)
		walls |= FORWARD_WALL_HIT;
	if (left.validSteps >= oldLeftSteps + STEP_INCRESE_THRESHOLD)
		walls |= LEFT_WALL_OPENED;
	if (right.validSteps >= oldRightSteps + STEP_INCRESE_THRESHOLD)
		walls |= RIGHT_WALL_OPENED;
}


//******************
// CheckFogOfWar
// TODO: have the map object check if ANY of the team entities have visited a tile about to be drawn, if not draw black,
// if so, then the map goes through all entities calling CheckFogOfWar, if ONE returns true then it'll stop sweep and draw bright,
// if none return true by the end of the sweep, then draw dim ( reduce sweep time by using a locational Potential_Visible_Set )
//******************
bool Entity::CheckFogOfWar(const eVec2 & point) const {
	eVec2 lineOfSight;

	lineOfSight = point - spriteCenter;
	return lineOfSight.LengthSquared() <= sightRange;
}

// FIXME: use the new GetMapIndex(eVec2) function to return point-wise snesor checks with (fewer?) operations
// Sets a sensor bit for every point within the entity's range in a non-traversible area ( 3 => no-walk tile, 0,1,2 => walk tile )
// self = true puts the sensors on the sprite's bounding box, 
// self = false puts them at touchRange off the bounding box
// horizontal updates the horizontally oriented sensors
// vertical updates the vertically oriented sensors
void Entity::CheckTouch(bool self, bool horizontal, bool vertical) {

	int tileSize = game->GetMap()->GetTileSize();
	int width = game->GetMap()->GetWidth();
	int height = game->GetMap()->GetHeight();

	// on-sprite checks
	if (self) {

		localTouch = 0;

		if (horizontal) {

			localTouch |= (RIGHT_TOP		* ((game->GetMap()->IndexValue((spritePos.x + size) / tileSize, (spritePos.y + 1) / tileSize) == COLLISION_TILE)		| ((spritePos.x + size + 1) > width)));
			localTouch |= (RIGHT_BOTTOM		* ((game->GetMap()->IndexValue((spritePos.x + size) / tileSize, (spritePos.y + size - 1) / tileSize) == COLLISION_TILE)	| ((spritePos.x + size + 1) > width)));
			
			localTouch |= (LEFT_BOTTOM		* ((game->GetMap()->IndexValue(spritePos.x / tileSize, (spritePos.y + size - 1) / tileSize) == COLLISION_TILE)			| (spritePos.x < 0)));
			localTouch |= (LEFT_TOP			* ((game->GetMap()->IndexValue(spritePos.x / tileSize, (spritePos.y + 1) / tileSize) == COLLISION_TILE)					| (spritePos.x < 0)));

		}

		if (vertical) {

			localTouch |= (TOP_LEFT		* ((game->GetMap()->IndexValue((spritePos.x + 1) / tileSize, spritePos.y / tileSize) == COLLISION_TILE)					| (spritePos.y < 0)));
			localTouch |= (TOP_RIGHT	* ((game->GetMap()->IndexValue((spritePos.x + size - 1) / tileSize, spritePos.y / tileSize) == COLLISION_TILE)			| (spritePos.y < 0)));

			localTouch |= (BOTTOM_RIGHT	* ((game->GetMap()->IndexValue((spritePos.x + size - 1) / tileSize, (spritePos.y + size) / tileSize) == COLLISION_TILE)	| ((spritePos.y + size + 1) > height)));
			localTouch |= (BOTTOM_LEFT	* ((game->GetMap()->IndexValue((spritePos.x + 1) / tileSize, (spritePos.y + size) / tileSize) == COLLISION_TILE)		| ((spritePos.y + size + 1) > height)));

		}

	} else { // functionally-ranged off-sprite checks

		oldTouch = touch;
		touch = 0;

		if (horizontal) {
			
			touch |= (RIGHT_TOP		* ((game->GetMap()->IndexValue((spritePos.x + size + touchRange) / tileSize, spritePos.y / tileSize) == COLLISION_TILE)				| ((spritePos.x + touchRange + size) > width)));
			touch |= (RIGHT_BOTTOM	* ((game->GetMap()->IndexValue((spritePos.x + size + touchRange) / tileSize, (spritePos.y + size) / tileSize) == COLLISION_TILE)	| ((spritePos.x + touchRange + size) > width)));

			touch |= (LEFT_TOP		* ((game->GetMap()->IndexValue((spritePos.x - touchRange) / tileSize, spritePos.y / tileSize) == COLLISION_TILE)					| ((spritePos.x - touchRange) < 0)));
			touch |= (LEFT_BOTTOM	* ((game->GetMap()->IndexValue((spritePos.x - touchRange) / tileSize, (spritePos.y + size) / tileSize) == COLLISION_TILE)			| ((spritePos.x - touchRange) < 0)));

		}

		if (vertical) {
		
			touch |= (TOP_RIGHT		* ((game->GetMap()->IndexValue((spritePos.x + size) / tileSize, (spritePos.y - touchRange) / tileSize) == COLLISION_TILE)			| ((spritePos.y - touchRange) < 0)));
			touch |= (TOP_LEFT		* ((game->GetMap()->IndexValue(spritePos.x / tileSize, (spritePos.y - touchRange) / tileSize) == COLLISION_TILE)					| ((spritePos.y - touchRange) < 0)));
			
			touch |= (BOTTOM_RIGHT	* ((game->GetMap()->IndexValue((spritePos.x + size) / tileSize, (spritePos.y + size + touchRange) / tileSize) == COLLISION_TILE)	| ((spritePos.y + touchRange + size) > height)));
			touch |= (BOTTOM_LEFT	* ((game->GetMap()->IndexValue(spritePos.x / tileSize, (spritePos.y + size + touchRange) / tileSize) == COLLISION_TILE)				| ((spritePos.y + touchRange + size) > height)));

		}
	}
}

// FIXME: should this belong to the Map class?
// Isolated check for overlap into non-traversable areas, and immediate sprite position correction
void Entity::CheckCollision(bool horizontal, bool vertical) {

	int tileSize = game->GetMap()->GetTileSize();
	int x1 = (int)(spritePos.x/tileSize);
	int x2 = (int)((spritePos.x+size)/tileSize);
	int y1 = (int)(spritePos.y/tileSize);
	int y2 = (int)((spritePos.y+size)/tileSize);
	int width = game->GetMap()->GetWidth();
	int height = game->GetMap()->GetHeight();
	int oldX = (int)spritePos.x;
	int oldY = (int)spritePos.y;

	// default map-edge collision
	if (horizontal) {

		if (spritePos.x < 0)
			spritePos.x = 0;
		else if (spritePos.x > (width - size))
			spritePos.x = (float)(width - 15);
	
	}

	if (vertical) {

		if (spritePos.y < 0)
			spritePos.y = 0;
		else if (spritePos.y > (height - size))
			spritePos.y = (float)(height - size);

	}
	
	if (spritePos.x != oldX || spritePos.y != oldY )
		return;

	// check the immediate boarder of the sprite
	CheckTouch(true, horizontal, vertical);

	// straight-on wall collision
	if (horizontal) {

		if (localTouch & (RIGHT_TOP | RIGHT_BOTTOM) && moveState & MOVE_RIGHT)
			spritePos.x = (float)(x2*tileSize - (size+1));
		else if (localTouch & (LEFT_TOP | LEFT_BOTTOM) && moveState & MOVE_LEFT)
			spritePos.x = (float)(x1*tileSize + tileSize);

	}

	if (vertical) {

		if (localTouch & (TOP_RIGHT | TOP_LEFT) && moveState & MOVE_UP)
			spritePos.y = (float)(y1*tileSize + tileSize);
		else if (localTouch & (BOTTOM_RIGHT | BOTTOM_LEFT) && moveState & MOVE_DOWN)
			spritePos.y = (float)(y2*tileSize - (size+1));

	}

	if (spritePos.x != oldX || spritePos.y != oldY)
		return;

	// wall-follower AI outside turn wall alignment
	CheckTouch(false, true, true);

	if (horizontal) {

		if (oldTouch & ~touch & (BOTTOM_LEFT | TOP_LEFT) && moveState & MOVE_RIGHT)
			spritePos.x = (float)(x1*tileSize);
		else if (oldTouch & ~touch & (BOTTOM_RIGHT | TOP_RIGHT) && moveState & MOVE_LEFT)
			spritePos.x = (float)(x2*tileSize + tileSize - (size+1));

	}

	if (vertical) {

		if (oldTouch & ~touch & (LEFT_BOTTOM | RIGHT_BOTTOM) && moveState & MOVE_UP)
			spritePos.y = (float)(y2*tileSize + tileSize - (size+1));
		else if (oldTouch & ~touch & (LEFT_TOP | RIGHT_TOP) && moveState & MOVE_DOWN)
			spritePos.y = (float)(y1*tileSize);

	}

}

// DEBUGGING
void Entity::PrintSensors() {

	char buffer[64];

	// touch sensors
	sprintf_s(buffer, "%s|%s", touch & TOP_LEFT ? "TL" : " ",
		touch & TOP_RIGHT ? "TR" : " ");
	game->DrawOutlineText(buffer, 100, 150, 255, 0, 0);

	sprintf_s(buffer, "%s|%s", touch & RIGHT_TOP ? "RT" : " ",
		touch & RIGHT_BOTTOM ? "RB" : " ");
	game->DrawOutlineText(buffer, 200, 200, 0, 255, 0);

	sprintf_s(buffer, "%s|%s", touch & BOTTOM_RIGHT ? "BR" : " ",
		touch & BOTTOM_LEFT ? "BL" : " ");
	game->DrawOutlineText(buffer, 150, 250, 0, 0, 255);

	sprintf_s(buffer, "%s|%s", touch & LEFT_BOTTOM ? "LB" : " ",
		touch & LEFT_TOP ? "LT" : " ");
	game->DrawOutlineText(buffer, 50, 200, 255, 255, 0);

	// FIXME: account for diagonal motion on printout
	// movement direction
	sprintf_s(buffer, "%s", moveState == MOVE_UP ? "UP" : moveState == MOVE_DOWN ? "DOWN" : moveState == MOVE_RIGHT ? "RIGHT" : moveState == MOVE_LEFT ? "LEFT" : " ");
	game->DrawOutlineText(buffer, 150, 300, 255, 255, 0);

	// sprite position (top-left corner of bounding box)
	sprintf_s(buffer, "%i, %i", (int)spritePos.x, (int)spritePos.y);
	game->DrawOutlineText(buffer, 150, 350, 255, 255, 255);

}

// returns a pointer to knownMap[r][c] closest
// to the given point on the knownMap (1:1 with tileMap)
// users must check for nullptr return value
int * Entity::KnownMapIndex(const eVec2 & point) {
	int row;
	int column;

	game->GetMap()->Index(point, row, column);
	if (row == INVALID_INDEX || column == INVALID_INDEX)
		return nullptr;
	else
		return &knownMap[row][column];
}

// return values: VISITED_TILE, UNKNOWN_TILE, INVALID_TILE
int Entity::KnownMapValue(int row, int column) const {

	if (row >= 0 && row < knownMapRows  && column >= 0 && column < knownMapCols)
		return knownMap[row][column];
	else
		return INVALID_TILE;
}

// returns the value in the element of the knownMap array
// nearest to the given point
// return values: VISITED_TILE, UNKNOWN_TILE, INVALID_TILE
int Entity::KnownMapValue(const eVec2 & point) {
	int * value;

	value = KnownMapIndex(point);
	if (value == nullptr)
		return INVALID_TILE;
	else
		return *value;
}

const eVec2 & Entity::Center() const {
	return spriteCenter;
}

void Entity::UpdateCenter() {
	spriteCenter.Set(spritePos.x + (sprite->w / 2), spritePos.y + (sprite->h / 2));
}

void Entity::DrawPixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0)
			return;
	}

	if (x >= surface->w || x < 0 || y >= surface->h || y < 0)
		return;

	Uint32 *buffer;
	Uint32 color;

	color = SDL_MapRGB(surface->format, r, g, b);

	buffer = (Uint32*)surface->pixels + y*surface->pitch / 4 + x;
	(*buffer) = color;

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);
}

void Entity::StopMoving() {
	forward.vector.Zero();
	left.vector.Zero();
	right.vector.Zero();
	moving = false;
}

// FIXME: ensure user-waypoints cannot be PushFront() if outside the map
// or on unreachable terrain
void Entity::AddUserWaypoint(const eVec2 & waypoint) {
	if (game->GetMap()->IsValid(waypoint)) {
		goals.PushFront(waypoint);
		UpdateWaypoint();
	}
}

// returns false if there's no waypoints available
bool Entity::UpdateWaypoint( bool getNext ) {
	switch (moveState) {
		case MOVE_TO_GOAL: {
			if ( getNext ) {
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
			if ( getNext ) 
				trail.PopFront();
			if (!CheckTrail()) {
				currentWaypoint = trail.Front();
				return true;
			} else if (!goals.IsEmpty()) {
				moveState = MOVE_TO_GOAL;
				currentWaypoint = goals.Back();
				return true;
			}
			return false;
		}
		default: 
			return false;
	}
}

// determine if the entity should fresh-start goal pathfinding
bool Entity::CheckTrail() {
	if (trail.IsEmpty()) {
		memset(&knownMap[0][0], UNKNOWN_TILE, MAX_MAP_ROWS * MAX_MAP_COLUMNS * sizeof(knownMap[0][0]));
		lastTrailTile = nullptr;
		return true;
	}
	return false;
}

// marks the currentTile as VISITED_Tile, clears out un-needed trail waypoints,
// and resets tiles around the current goal waypoint to UNKNOWN_TILE
void Entity::UpdateKnownMap() {
	int * goalTile;
	int row, column;
	int startRow, startCol;
	int endRow, endCol;
	int tileResetRange = 0;		// size of the box around the goal to set tiles to UNKNOWN_TILE
	static const float tileSize = (float)game->GetMap()->GetTileSize();

	// update the newly visited tile and put it into the trailMap
	if (currentTile != nullptr) 
		*currentTile = VISITED_TILE;

	// fill-box of tiles at the tileResetRange centered on **the current goal waypoint** to to reset the knownMap:
	if ( !goals.IsEmpty() ) {
		tileResetRange = (int)( (goals.Back() - spriteCenter).Length() / (tileSize * 2) );

		goalTile = KnownMapIndex(goals.Back());
		if (goalTile == nullptr)
			return;

		// knownMap indexes of goalTile, that is goalTile == &knownMap[row][column]
		row			= (goalTile - (int *)&knownMap[0][0]) / MAX_MAP_COLUMNS;
		column		= (goalTile - (int *)&knownMap[0][0]) % MAX_MAP_COLUMNS;

		// set initial bounding box top-left and bottom-right indexes within knownMap
		startRow	= row - (tileResetRange / 2);
		startCol	= column - (tileResetRange / 2);
		endRow		= row + (tileResetRange / 2);
		endCol		= column + (tileResetRange / 2);

		// range-correct the bounding box
		// snap bounding box rows & columns within range of the tileMap area
		if (startRow < 0)
			startRow = 0;
//		else if (startRow >= knownMapRows || startCol >= knownMapCols)	// starts below or to the right of tileMap area
//			return;														// never occurs because goalTile is always on the tileMap
		if (startCol < 0)
			startCol = 0;

		if (endRow >= knownMapRows)
			endRow = knownMapRows - 1;
//		else if (endRow < 0 || endCol < 0)								// ends above or to the left of tileMap area
//			return;														// never occurs because goalTile is always on the tileMap
		if (endCol >= knownMapCols)
			endCol = knownMapCols - 1;

		// reset tiles within the bounding box
		row = startRow;
		column = startCol;
		while ( 1 ) {
			knownMap[row][column] = UNKNOWN_TILE;

			column++;
			if (column > endCol) {
				column = startCol; 
				row++; 
			} 
			if (row > endRow)
				break;
		}
	}

/*
// FREEHILL begin currentTile knownMap reset box test
	// perimiter-box of tiles at the tileResetRange centered on **the currentTile** to to reset the knownMap:
	// column of tiles to the left
	for (i = -tileResetRange, j = -tileResetRange; j <= tileResetRange; j++) {
		offset = i * MAX_MAP_SIZE + j;

		resetTile = currentTile + offset;
		if (resetTile >= &knownMap[0][0] && resetTile <= knownMapEnd)
			 *resetTile = UNKNOWN_TILE;
	}

	// column of tiles to the right
	for (i = tileResetRange, j = -tileResetRange; j <= tileResetRange; j++) {
		offset = i * MAX_MAP_SIZE + j;

		resetTile = currentTile + offset;
		if (resetTile >= &knownMap[0][0] && resetTile <= knownMapEnd)
			*resetTile = UNKNOWN_TILE;
	}

	// row of tiles above
	for (i = -tileResetRange, j = -tileResetRange; i <= tileResetRange; i++) {
		offset = i * MAX_MAP_SIZE + j;

		resetTile = currentTile + offset;
		if (resetTile >= &knownMap[0][0] && resetTile <= knownMapEnd)
			*resetTile = UNKNOWN_TILE;
	}

	// row of tiles below
	for (i = -tileResetRange, j = tileResetRange; i <= tileResetRange; i++) {
		offset = i * MAX_MAP_SIZE + j;

		resetTile = currentTile + offset;
		if (resetTile >= &knownMap[0][0] && resetTile <= knownMapEnd)
			*resetTile = UNKNOWN_TILE;
	}
// FREEHILL begin currentTile knownMap reset box test
*/
	// pop all trail waypoints that no longer fall on VISITED_TILEs
	while (!trail.IsEmpty()) {
		if (KnownMapValue(trail.Back()) == UNKNOWN_TILE)
			trail.PopBack();
		else
			break;
	}
}

void Entity::UpdatePosition() {
	spritePos += forward.vector*speed;
	UpdateCenter();
}

void Entity::SetPosition(const eVec2 & point) {
	spritePos.Set(point.x, point.y);
	UpdateCenter();
}
