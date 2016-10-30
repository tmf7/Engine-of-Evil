#include "Entity.h"
#include "Game.h"

// FIXME: these variable names are too verbose and not specific enough
Entity::Entity() {
	speed = MAX_SPEED;
	size = 15;
	sightRange = 128.0f;
	goalRange = speed;
	rotationQuat_Z.Set(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));
	touch.reach = 1;
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
	knownMap.SetInvalidCell((byte_t)-1);		// largest possible unsigned char == 255
	knownMap.SetCellWidth(game->GetMap()->TileMap().CellWidth());	// FIXME(?): GetMap() or game might be null, use reference
	knownMap.SetCellHeight(game->GetMap()->TileMap().CellHeight());
	knownMap.SetRowLimit(game->GetMap()->TileMap().RowLimit());
	knownMap.SetColumnLimit(game->GetMap()->TileMap().ColumnLimit());
	knownMap.ClearAllCells();

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
void Entity::Spawn() {
	eVec2 testPoint;
	int i, j;
	int radius = 1;
	bool entity_placed = false;

	// Check the top-left corner first
	testPoint.Set(5.0f, 5.0f);
	if (game->GetMap()->IsValid( testPoint ) ) {

		spritePos = testPoint;
		UpdateCenter();
		currentTile = knownMap.Index(0, 0);
		lastTrailTile = nullptr;
		return;
	}

	// Expand the search
	while (!entity_placed) {

		// down from the top
		for (i = radius, j = 0; j <= radius; j++) {

			testPoint.Set((float)(i * knownMap.CellWidth() + 5), (float)(j * knownMap.CellHeight() + 5));
			if (game->GetMap()->IsValid( testPoint ) ) {
				spritePos = testPoint;
				entity_placed = true;
				break;
			}
		}

		// TODO: test to ensure this loop isnt entered if (entity_placed)
		// in from the bottom of the last search
		for (i = radius - 1, j = radius; i >= 0 && !entity_placed; i--) {

			testPoint.Set((float)(i * knownMap.CellWidth() + 5), (float)(j * knownMap.CellHeight() + 5));
			if (game->GetMap()->IsValid( testPoint ) ) {
				spritePos = testPoint;
				entity_placed = true;
				break;
			}
		}

		radius++;
	}
	UpdateCenter();
	currentTile = knownMap.Index(i, j);
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

// BEGIN FREEHILL DEBUG knownMap memset test
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_R])
		knownMap.ClearAllCells();
// END FREEHIL DEBUG knownMap memset test

	// change/initialize the movement type under specific conditions
	// TODO: give this a broader functionality beyond pure AI movement (ie player input)
	// Ultimate Goal: select a pathfinding type (eg: waypoint+obstacle avoid, A* optimal path, wall follow, Area awareness, raw compass?, etc)
	static bool movementInitialized = false;

	if (!movementInitialized) {
		moveState = MOVE_TO_GOAL;
		StopMoving();
		Move = &Entity::WaypointFollow;
		movementInitialized = true;
	} else if (!movementInitialized) {
		moveState = MOVE_RIGHT;
		touch.Clear();
		Move = &Entity::WallFollow;
		movementInitialized = true;
	}

	((*this).*Move)();

	// draw all waypoints
	// FIXME: draws regardless if they're visible
	node = 0;
	while(node < goals.Size()) {
		debugWaypoint = goals.FromBack(node);

		destRect.x = (int)(debugWaypoint.x - game->GetMap()->camera.position.x);
		destRect.y = (int)(debugWaypoint.y - game->GetMap()->camera.position.y);
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
		node++;
		
	}

	node = 0;
	while (node < trail.Size()) {
		debugWaypoint = trail.FromFront(node);

		destRect.x = (int)(debugWaypoint.x - game->GetMap()->camera.position.x);
		destRect.y = (int)(debugWaypoint.y - game->GetMap()->camera.position.y);
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
		node++;
	}

	// draw sprite 
	// FIXME: draws regardless if it's visible
	destRect.x = (int)(spritePos.x - game->GetMap()->camera.position.x);
	destRect.y = (int)(spritePos.y - game->GetMap()->camera.position.y);
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

			collisionX = (int)(spriteCenter.x + (collisionRadius*debugVector.x) - game->GetMap()->camera.position.x);
			collisionY = (int)(spriteCenter.y + (collisionRadius*debugVector.y) - game->GetMap()->camera.position.y);
			DrawPixel(game->GetBuffer(), collisionX, collisionY, color[0], color[1], color[2]);
		}
		debugVector = rotationQuat_Z*debugVector;	// rotate counter-clockwise
		rotationAngle += ROTATION_INCREMENT;
	}
	// FREEHILL END DEBUG COLLISION CIRCLE

	PrintSensors();
}

void Entity::WaypointFollow() {
	byte_t * checkTile;

	// don't move without a waypoint
	if (!UpdateWaypoint())
		return;

	UpdateVector();
	UpdatePosition();

	if ( spriteCenter.Compare(currentWaypoint, goalRange) ) {
		StopMoving();
		UpdateWaypoint(true);
	}

	// mark the tile to help future movement decision
	checkTile = knownMap.Index(spriteCenter);
	if ( checkTile != nullptr && checkTile != currentTile ) {
		UpdateKnownMap();
		currentTile = checkTile;
	}

}

void Entity::WallFollow() {

	oldMoveState = moveState;

	spritePos.x -= speed * (moveState == MOVE_LEFT);
	spritePos.x += speed * (moveState == MOVE_RIGHT);

	CheckCollision();
	UpdateCenter();

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

			if (moveState != oldMoveState)
				return;

			break;
		}

		// rinse, repeat
		case MOVE_LEFT: {

			if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) || 
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.BOTTOM_RIGHT))
				moveState = MOVE_DOWN;

			else if ((touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT) || 
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.TOP_RIGHT))
				moveState = MOVE_UP;

			if (moveState != oldMoveState)
				return;

			break;
		}
	}

	spritePos.y -= speed * (moveState == MOVE_UP);
	spritePos.y += speed * (moveState == MOVE_DOWN);

	CheckCollision();
	UpdateCenter();

	switch (moveState) {

		case MOVE_UP: {

			if ((touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM) || 
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.RIGHT_BOTTOM))
				moveState = MOVE_RIGHT;

			else if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) || 
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.LEFT_BOTTOM))
				moveState = MOVE_LEFT;

			if (moveState != oldMoveState)
				return;

			break;
		}

		case MOVE_DOWN: {

			if ((touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP) || 
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.RIGHT_TOP))
				moveState = MOVE_RIGHT;

			else if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) || 
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.LEFT_TOP))
				moveState = MOVE_LEFT;

			if (moveState != oldMoveState)
				return;

			break;
		}
	}	
}

//******************
// UpdateVector
// Determines the optimal movement vector to reach the current waypoint
//******************
void Entity::UpdateVector() {
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

	// modulate entity speed if at least this close to a waypoint
	static const float goalRangeSqr = MAX_SPEED * MAX_SPEED * MAX_STEPS * MAX_STEPS;

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
	if (moveState == MOVE_TO_GOAL) {							// FIXME/BUG(?): stays slow if it was slow and switched to MOVE_TO_TRAIL
		distToWaypointSqr = waypoint.vector.LengthSquared();	// TODO: potentially use this value in UpdateKnownMap();

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
		if ( CheckVectorPath( spriteCenter+(test.vector*speed), test ) ) {
			if (CheckVectorPath( spriteCenter+(waypoint.vector*speed), waypoint) )
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
// CheckVectorPath
// determines the state of the sprite's position for the next few frames
// return true if a future position using along is near the waypoint
//******************
bool Entity::CheckVectorPath(eVec2 from, decision_t & along) {
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
		if (knownMap.Cell(from) == UNKNOWN_TILE)
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
// CheckWalls
// assigns the vectors perpendicular to the forward vector
// and checks if the range along them has significantly changed
//**************
void Entity::CheckWalls(int & walls) {
	int oldLeftSteps = left.validSteps;
	int oldRightSteps = right.validSteps;

	right.vector.Set(forward.vector.y, -forward.vector.x);	// forward rotated 90 degrees clockwise
	left.vector.Set(-forward.vector.y, forward.vector.x);	// forward rotated 90 degrees counter-clockwise

	CheckVectorPath(spriteCenter + (forward.vector*speed), forward);
	CheckVectorPath(spriteCenter + (left.vector*speed), left);
	CheckVectorPath(spriteCenter + (right.vector*speed), right);

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

// Sets a sensor bit for every point within the entity's range in a non-traversable area
// self == true puts the sensors on the sprite's bounding box, 
// self == false puts them at touchRange off the bounding box
// --the entity effectively has 16 touch sensors--
void Entity::CheckTouch(bool self) {
	
	if (self) {	// on-sprite checks
		touch.local.Clear();

		// horizontally oriented sensors
		touch.local.RIGHT_TOP		= !game->GetMap()->IsValid( eVec2(spritePos.x + size, spritePos.y + 1) );
		touch.local.RIGHT_BOTTOM	= !game->GetMap()->IsValid( eVec2(spritePos.x + size, spritePos.y + size - 1) );
		touch.local.LEFT_BOTTOM		= !game->GetMap()->IsValid( eVec2(spritePos.x, spritePos.y + size - 1) );
		touch.local.LEFT_TOP		= !game->GetMap()->IsValid( eVec2(spritePos.x, spritePos.y + 1) );

		// vertically oriented sensors
		touch.local.TOP_LEFT		= !game->GetMap()->IsValid( eVec2(spritePos.x + 1, spritePos.y) );
		touch.local.TOP_RIGHT		= !game->GetMap()->IsValid( eVec2(spritePos.x + size - 1, spritePos.y) );
		touch.local.BOTTOM_RIGHT	= !game->GetMap()->IsValid( eVec2(spritePos.x + size - 1, spritePos.y + size) );
		touch.local.BOTTOM_LEFT		= !game->GetMap()->IsValid( eVec2(spritePos.x + 1, spritePos.y + size) );

	} else { // ranged off-sprite checks
		touch.oldRanged = touch.ranged;
		touch.ranged.Clear();

		// horizontally oriented sensors
		touch.ranged.RIGHT_TOP		= !game->GetMap()->IsValid( eVec2(spritePos.x + size + touch.reach, spritePos.y) );
		touch.ranged.RIGHT_BOTTOM	= !game->GetMap()->IsValid( eVec2(spritePos.x + size + touch.reach, spritePos.y + size) );
		touch.ranged.LEFT_TOP		= !game->GetMap()->IsValid( eVec2(spritePos.x - touch.reach, spritePos.y) );
		touch.ranged.LEFT_BOTTOM	= !game->GetMap()->IsValid( eVec2(spritePos.x - touch.reach, spritePos.y + size) );

		// vertically oriented sensors
		touch.ranged.TOP_RIGHT		= !game->GetMap()->IsValid( eVec2(spritePos.x + size, spritePos.y - touch.reach) );
		touch.ranged.TOP_LEFT		= !game->GetMap()->IsValid( eVec2(spritePos.x, spritePos.y - touch.reach) );
		touch.ranged.BOTTOM_RIGHT	= !game->GetMap()->IsValid( eVec2(spritePos.x + size, spritePos.y + size + touch.reach) );
		touch.ranged.BOTTOM_LEFT	= !game->GetMap()->IsValid( eVec2(spritePos.x, spritePos.y + size + touch.reach) );
	}
}

// TODO(?): have CheckTouch return a bool if any of the requested sensors are triggered
// TODO: give Entity a Rectangle object to act as its bounding box
// FIXME: should this belong to the Map class? or a Collision class? or stay here?
// Isolated check for overlap into non-traversable areas, and immediate sprite position correction
void Entity::CheckCollision() {

	// FIXME: the goal of this function is to snap to the most appropriate tile edge 
	// should an overlap occur (or in the case of wall-follower lack of overlap on an outside turn)
	// NOTE: regardless of how the entity has moved, always collide each coordinate individually
	// ie: first x-correct, then y-correct
	int x1			= (int)(spritePos.x)		/ knownMap.CellWidth();
	int x2			= (int)(spritePos.x+size)	/ knownMap.CellWidth();
	int y1			= (int)(spritePos.y)		/ knownMap.CellHeight();
	int y2			= (int)(spritePos.y+size)	/ knownMap.CellHeight();
	eVec2 oldSpritePos	= spritePos;

	// default map-edge collision
	if (spritePos.x < 0)
		spritePos.x = 0;
	else if (spritePos.x > (knownMap.Width() - size)) // FIXME(?): if width < size, then spritePos.x becomes extremly large
		spritePos.x = (float)(knownMap.Width() - size);

	if (spritePos.y < 0)
		spritePos.y = 0;
	else if (spritePos.y > (knownMap.Height() - size))
		spritePos.y = (float)(knownMap.Height() - size);

	// check the immediate boarder of the sprite
	CheckTouch(true);

	// straight-on wall collision
	switch(moveState) {
		case MOVE_RIGHT: {
			if (touch.local.RIGHT_TOP || touch.local.RIGHT_BOTTOM)
				spritePos.x = (float)(x2 * knownMap.CellWidth() - (size + 1));
			break;
		}
		case MOVE_LEFT: {
			if (touch.local.LEFT_TOP || touch.local.LEFT_BOTTOM)
				spritePos.x = (float)(x1 * knownMap.CellWidth() + knownMap.CellWidth());
			break;
		}
		case MOVE_UP: {
			if (touch.local.TOP_RIGHT || touch.local.TOP_LEFT)
				spritePos.y = (float)(y1 * knownMap.CellHeight() + knownMap.CellHeight());
			break;
		}
		case MOVE_DOWN: {
			if (touch.local.BOTTOM_RIGHT || touch.local.BOTTOM_LEFT)
				spritePos.y = (float)(y2 * knownMap.CellHeight() - (size + 1));
			break;
		}
	}

	if (spritePos != oldSpritePos)
		return;

	// check just off the sprite
	CheckTouch(false);

	// wall-follower AI outside turn wall alignment (essentially backwards collision)
	switch (moveState) {
		case MOVE_RIGHT: {
			if ((touch.oldRanged.BOTTOM_LEFT && !touch.ranged.BOTTOM_LEFT) || 
				(touch.oldRanged.TOP_LEFT && !touch.ranged.TOP_LEFT))
				spritePos.x = (float)(x1 * knownMap.CellWidth());
			break;
		}
		case MOVE_LEFT: {
			if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) ||
				(touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT))
				spritePos.x = (float)(x2 * knownMap.CellWidth() + knownMap.CellWidth() - (size + 1));
			break;
		}
		case MOVE_UP: {
			if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) || 
				(touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM))
				spritePos.y = (float)(y2 * knownMap.CellHeight() + knownMap.CellHeight() - (size + 1));
			break;
		}
		case MOVE_DOWN: {
			if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) ||
				(touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP))
				spritePos.y = (float)(y1 * knownMap.CellHeight());
			break;
		}
	}
}

// DEBUGGING
void Entity::PrintSensors() {

	char buffer[64];

	// touch sensors
	sprintf_s(buffer, "%s|%s", touch.ranged.TOP_LEFT ? "TL" : " ",
		touch.ranged.TOP_RIGHT ? "TR" : " ");
	game->DrawOutlineText(buffer, 100, 150, 255, 0, 0);

	sprintf_s(buffer, "%s|%s", touch.ranged.RIGHT_TOP ? "RT" : " ",
		touch.ranged.RIGHT_BOTTOM ? "RB" : " ");
	game->DrawOutlineText(buffer, 200, 200, 0, 255, 0);

	sprintf_s(buffer, "%s|%s", touch.ranged.BOTTOM_RIGHT ? "BR" : " ",
		touch.ranged.BOTTOM_LEFT ? "BL" : " ");
	game->DrawOutlineText(buffer, 150, 250, 0, 0, 255);

	sprintf_s(buffer, "%s|%s", touch.ranged.LEFT_BOTTOM ? "LB" : " ",
		touch.ranged.LEFT_TOP ? "LT" : " ");
	game->DrawOutlineText(buffer, 50, 200, 255, 255, 0);

	// FIXME: account for diagonal motion on printout
	// movement direction
	sprintf_s(buffer, "%s", moveState == MOVE_UP ? "UP" : moveState == MOVE_DOWN ? "DOWN" : moveState == MOVE_RIGHT ? "RIGHT" : moveState == MOVE_LEFT ? "LEFT" : " ");
	game->DrawOutlineText(buffer, 150, 300, 255, 255, 0);

	// sprite position (top-left corner of bounding box)
	sprintf_s(buffer, "%i, %i", (int)spritePos.x, (int)spritePos.y);
	game->DrawOutlineText(buffer, 150, 350, 255, 255, 255);

}

// FIXME: make this a member funciton of a Graphics class
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
		knownMap.ClearAllCells();
		lastTrailTile = nullptr;
		return true;
	}
	return false;
}

// marks the currentTile as VISITED_TILE, clears out un-needed trail waypoints,
// and resets tiles around the current goal waypoint to UNKNOWN_TILE
void Entity::UpdateKnownMap() {
	byte_t * goalTile;
	int row, column;
	int startRow, startCol;
	int endRow, endCol;
	int tileResetRange = 0;		// size of the box around the goal to set tiles to UNKNOWN_TILE

	// update the newly visited tile and put it into the trailMap
	if (currentTile != nullptr) 
		*currentTile = VISITED_TILE;

	// solid-box of tiles at the tileResetRange centered on **the current goal waypoint** to to reset the knownMap:
	if ( !goals.IsEmpty() ) {
		tileResetRange = (int)( (goals.Back() - spriteCenter).Length() / (knownMap.CellWidth() * 2) );

		goalTile = knownMap.Index(goals.Back());
		if (goalTile == nullptr)
			return;

		// knownMap indexes of goalTile, that is goalTile == &knownMap[row][column]
		row			= (goalTile - knownMap.Index(0, 0)) / MAX_MAP_COLUMNS;
		column		= (goalTile - knownMap.Index(0, 0)) % MAX_MAP_COLUMNS;

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

		if (endRow >= knownMap.RowLimit())
			endRow = knownMap.RowLimit() - 1;
//		else if (endRow < 0 || endCol < 0)								// ends above or to the left of tileMap area
//			return;														// never occurs because goalTile is always on the tileMap
		if (endCol >= knownMap.ColumnLimit())
			endCol = knownMap.ColumnLimit() - 1;

		// reset tiles within the bounding box
		row = startRow;
		column = startCol;
		while ( row <= endRow ) {
			*knownMap.Index(row, column) = UNKNOWN_TILE;

			column++;
			if (column > endCol) {
				column = startCol; 
				row++; 
			} 
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
		if (knownMap.Cell(trail.Back()) == UNKNOWN_TILE)
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
