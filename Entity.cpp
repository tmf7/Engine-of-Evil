#include "Game.h"

Entity::Entity() {

}

Entity::~Entity() {
	
}

// TODO: insert use of the sourceRect for initial frame of animation
bool Entity::Init(char fileName[], bool key, Game *const g) {

	SDL_Surface* surface = NULL;

	if (!g)
		return false;

	game = g;

	if (!fileName[0])
		return false;

	surface = SDL_LoadBMP(fileName);

	if (!surface)
		return false;

	sprite = SDL_ConvertSurface(surface, game->GetBuffer()->format, 0);

	frameDelayCount = 0;

	SDL_FreeSurface(surface);
	surface = NULL;

	if (sprite && key) {

		Uint32 colorKey = SDL_MapRGB(sprite->format, 255, 0, 255);
		SDL_SetColorKey(sprite, SDL_TRUE, colorKey);
	}

	speed = 10;
	size = 15;
	sightRange = 128;
	sight.w = sightRange;
	sight.h = sightRange;
	touchRange = 1;
	waypointRange = speed*speed/4; // squared range of speed/2 to speed up waypoint range finding

	VectorClear(movementVector);

	collisionRadius = (float)((sprite->w * sprite->w) + (sprite->h * sprite->h));
	collisionRadius = SDL_sqrtf(collisionRadius);
	collisionRadius /= 2.0f;

	currentWaypoint = -1;
	userWaypoint = -1;
	maxWaypoint = 0;

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
	moveState = MOVE_RIGHT;
	oldTouch = touch = 0;

	// Check the top-left corner first
	if (game->GetMap()->GetMapIndex(0,0) < 3) {

		spritePos.x = 0;
		spritePos.y = 0;
		return;
	}

	// Expand the search
	while (!entity_placed) {

		// down from the top
		for (i = radius, j = 0; j <= radius && !entity_placed; j++) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {
				spritePos.x = i*tileSize;
				spritePos.y = j*tileSize;
				entity_placed = true;
			}
		}

		// TODO: // test to ensure this loop isnt entered if (entity_placed)
		// in from the bottom of the last search
		for (i = radius - 1, j = radius; i >= 0 && !entity_placed; i--) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {

				spritePos.x = i*tileSize;
				spritePos.y = j*tileSize;
				entity_placed = true;
			}
		}

		radius++;
	}
}

// TODO: add animation frame selection here
void Entity::Update() {

	SDL_Rect destRect;
//	int columns = image->w / width;

	Move();

	// Account for camera movement when drawing
	destRect.x = spritePos.x - game->GetMap()->GetCamera()->x;
	destRect.y = spritePos.y - game->GetMap()->GetCamera()->y;

/*
	if (frameDelayCount > frameDelay){

		frameDelayCount = 0;
		currentFrame++;
	}

	if (currentFrame > lastFrame)
		currentFrame = firstFrame;

//	SDL_Rect sourceRect;
//	sourceRect.y = 0;			// (frame / columns)*height;
//	sourceRect.x = (frame%columns)*width;
//	sourceRect.w = width;
//	sourceRect.h = height;

//	SDL_BlitSurface(sprite, &sourceRect, game->GetBuffer, &destRect);
*/
	// FIXME: confirm that destRect is within view of the camera
	SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);

	// draw the waypoints
	for (int i = 0; i < maxWaypoint; i++) {

		destRect.x = waypoints[i].x - game->GetMap()->GetCamera()->x;
		destRect.y = waypoints[i].y - game->GetMap()->GetCamera()->y;
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
	}
}

// Ultimate Goal: select a pathfinding type (eg: compass, endpoint+obstacle adjust, waypoints+minipaths, wall follow, Area awareness, etc)
// Gather sensor information and decide how to move
void Entity::Move() {

	//CheckFogOfWar();	// also uncomment the Update() code in Map.cpp
	
	// FIXME: second condition needs some work to ensure the sprite follows the full trail, then stops/waits
	if ((userWaypoint < 0) || (currentWaypoint == userWaypoint) )
		return;

	// FIXME: ignores CheckLOS too much... (goes off map in one direction)
	// ignore checking LOS if a movementVector is already established, wait until a better position is acquired to check.
	if ( (!movementVector[0] && !movementVector[1] && !movementVector[2]) || 
		(GetCenterX() == maxMovePoint.x && GetCenterY() == maxMovePoint.y) )
		CheckLineOfSight();

	spritePos.x = (int)(spritePos.x + speed*movementVector[0]);
	spritePos.y = (int)(spritePos.y + speed*movementVector[1]);

	if (FastLength(&spritePos, &waypoints[currentWaypoint]) <= waypointRange) {

		spritePos.x = waypoints[currentWaypoint].x;
		spritePos.y = waypoints[currentWaypoint].y;
		VectorClear(movementVector);
		currentWaypoint++;
	}

	/* 
	////////////////////////////////////////
	//BEGIN WALL FOLLOW MOVEMENT ALGORITHM//
	////////////////////////////////////////

	oldMoveState = moveState;

	x -= speed * ((moveState & MOVE_LEFT) > 0);
	x += speed * ((moveState & MOVE_RIGHT) > 0);

	CollisionCheck(true, false);

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

	CollisionCheck(false, true);

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
// CheckLineOfSight
// Determines the optimal movement vector to reach a given waypoint coordinate on the map
// TODO(?): plot several paths and select the *optimal*(shortest?) one and/or randomize the final BEFORE MOVING
// TODO: add functionality to check if the sprite STARTS in water (then set a random vector and loop-check TOUCH sensors until clear)
// TODO(?): use a true spatial Trace algorithm (with an endpoint and cross-section)
//******************
void Entity::CheckLineOfSight() {

	quat_s rotationQuat;		// quaternion formated rotationAxis and rotationAngle
	quat_s testQuat;			// quaternion formated testVector
	vec3_t waypointVector;		// from the sprite to the next waypoint
	vec3_t testVector;			// tested for optimal travel decision
	vec3_t bestVector;			// the highest weight < 5
	float bestWeight;			// maximum decision weight == 5 sets movementVector = vector
								// retains highest weight < 5 in the event of a full 360 degree sweep without a 5
	float distToWaypoint;		// **currently not fully utilized for checks, but a potentially useful metric**
	float rotationAngle;		// amount to rotate the testVector away from the waypointVector (CW or CCW)
	point_s testSpritePos;
	point_s testSpriteCenter;
	point_s testPoint;
	float weight_mod;
	int weight;
	int tileSize; 
	int width;  
	int height; 

	tileSize = game->GetMap()->GetTileSize();
	width = game->GetMap()->GetWidth()*tileSize;
	height = game->GetMap()->GetHeight()*tileSize;

	testSpritePos.x = spritePos.x;
	testSpritePos.y = spritePos.y;
	testSpriteCenter.x = GetCenterX();
	testSpriteCenter.y = GetCenterY();
	
	distToWaypoint = VectorNormalize2(&testSpriteCenter, &waypoints[currentWaypoint], waypointVector);
	VectorCopy(waypointVector, testVector);

	rotationAngle = 0;
	VectorClear(bestVector);
	bestWeight = 0;

	// FIXME: some rounding errors(?) may eventually push the sprite into no-walk, which may cause it to go straight forever
	// FIXME: rounding errors? (float to int)

	// collision prediction
	while (1) { // until the bestVector for the movementVector is found in a 360 degree scan

		weight = 0;
		
		// FIXME: either there is a cumulative rounding error that causes maxMovePoint != spritePos after weight # of frames,
		// or my equation for the testPoints is wrong. I'm leaning towards a rounding error based off spriteCenter
		// FIXME: ***START HERE*** its always choosing the same starting movementVector, and anything more than ONE step along
		// causes the final tested position to be ignored (still possible rounding errors between spriteCenter to spritePos checks)
		while (1) {// TODO: check if the waypoint landed somewhere in a set, or between sets (along the swept area), and immediatly set THAT as the movementVector

			// forward test point (starts on sprite)
			testPoint.x = (int)(testSpriteCenter.x + collisionRadius*testVector[0]);
			testPoint.y = (int)(testSpriteCenter.y + collisionRadius*testVector[1]);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) || 
				(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;

			// forward test point rotated counter-clockwise 90 degrees
			testPoint.x = (int)(testSpriteCenter.x + collisionRadius*testVector[1]);
			testPoint.y = (int)(testSpriteCenter.y - collisionRadius*testVector[0]);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) ||
				(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;

			// forward test point rotated clockwise 90 degrees
			testPoint.x = (int)(testSpriteCenter.x - collisionRadius*testVector[1]);
			testPoint.y = (int)(testSpriteCenter.y + collisionRadius*testVector[0]);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) ||
				(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;

			weight++;

			if (weight == MAX_LOS_WEIGHT)
				break;

			testSpritePos.x = (int)(testSpritePos.x + speed*testVector[0]);
			testSpritePos.y = (int)(testSpritePos.y + speed*testVector[1]);
			testSpriteCenter.x = testSpritePos.x + (sprite->w / 2);
			testSpriteCenter.y = testSpritePos.y + (sprite->h / 2);
		}

		weight_mod = DotProduct(waypointVector, testVector);

		if ( (weight+weight_mod) > bestWeight) {

			bestWeight = weight + weight_mod;
			VectorCopy(testVector, bestVector);
			maxMovePoint.x = testSpriteCenter.x;
			maxMovePoint.y = testSpriteCenter.y;
		}
/*
		// rotation direction decision bias, and quicker exit
		// FIXME: improve this shortcut exit statement
		if (weight == MAX_LOS_WEIGHT) {

			VectorCopy(bestVector, movementVector);
			break;
		}
*/

		do {
			rotationAngle += 1.0f;

			if (rotationAngle >= 360) {

				if (bestWeight <= 2)
					VectorClear(movementVector);
				else
					VectorCopy(bestVector, movementVector);
				return;
			}

			rotationQuat.vector[0] = 0;
			rotationQuat.vector[1] = 0;
			rotationQuat.vector[2] = SDL_sinf(DEG2RAD(rotationAngle) / 2.0f);
			rotationQuat.scalar = SDL_cosf(DEG2RAD(rotationAngle) / 2.0f);

			VectorCopy(waypointVector, testQuat.vector);
			testQuat.scalar = 0;

			// unit length quaternion => inverse == conjugate
			// 2D rotation about the z-axis => only negate z-axis of quat vector
			// v' = q*v*q^-1
			QuatProduct(&rotationQuat, &testQuat, &testQuat);
			rotationQuat.vector[2] = -rotationQuat.vector[2];
			QuatProduct(&testQuat, &rotationQuat, &testQuat);

			// extract rotated testVector from testQuat
			VectorCopy(testQuat.vector, testVector);

		} while (DotProduct(testVector, movementVector) < 0);	// avoid testVectors that backtrack
	}
}

//******************
// CheckFogOfWar
// highlight the subset of tiles from the tileMap that are currently visible to the entity ( ie a '2' in the array)
// and convert the *old* tiles no longer within sight range (that had a '2') to "known" (ie a '1' in the array)
// the 2's will be used by the map to draw bright tiles, the 1's will be grey tiles, the 0's will be black/background
//******************
void Entity::CheckFogOfWar() {

	// set the fog of war properties
	sight.x = spritePos.x - (sightRange / 2);
	sight.y = spritePos.y - (sightRange / 2);

	// FIXME: inefficient allocation of pre-existing data as well as repetitive function calls to (currently) constant data
	int tileSize = game->GetMap()->GetTileSize();
	int width = game->GetMap()->GetWidth();
	int height = game->GetMap()->GetHeight();

	// continue just beyond the sightRange to account for 2s to 1s visibility transitions
	int rows = (sight.h / tileSize) + 2;
	int columns = (sight.w / tileSize) + 2;
	int i, j, startI, startJ;

	startI = sight.x / tileSize;
	startJ = sight.y / tileSize;

	// FIXME: rough reset of knownMap; should set all 2s to 1s and keep the rest 0
	//memset(knownMap, 0, MAX_MAP_SIZE*MAX_MAP_SIZE*sizeof(int));

	for (i = startI; i < startI + columns; i++) {

		for (j = startJ; j < startJ + rows; j++) {

			if (i >= 0 && i < width && j >= 0 && j < height) {

				if ( i > startI && i < (startI + columns - 1) &&
					j > startJ && j < (startJ + rows - 1) ) 
					knownMap[i][j] = 2;
				else
					knownMap[i][j] = 1;
			}
		}
	}
}

// Sets a sensor bit for every point within the entity's range in a non-traversible area ( 3 => no-walk tile, 0,1,2 => walk tile )
// self = true puts the sensors on the sprite's bounding box, 
// self = false puts them at touchRange off the bounding box
// horizontal updates the horizontally oriented sensors
// vertical updates the vertically oriented sensors
void Entity::CheckTouch(bool self, bool horizontal, bool vertical) {

	int tileSize = game->GetMap()->GetTileSize();
	int width = game->GetMap()->GetWidth()*tileSize;
	int height = game->GetMap()->GetHeight()*tileSize;

	// on-sprite checks
	if (self) {

		localTouch = 0;

		if (horizontal) {

			localTouch |= (RIGHT_TOP		* ((game->GetMap()->GetMapIndex((spritePos.x + size) / tileSize, (spritePos.y + 1) / tileSize) == 3)		| ((spritePos.x + size + 1) > width)));
			localTouch |= (RIGHT_BOTTOM		* ((game->GetMap()->GetMapIndex((spritePos.x + size) / tileSize, (spritePos.y + size - 1) / tileSize) == 3)	| ((spritePos.x + size + 1) > width)));
			
			localTouch |= (LEFT_BOTTOM		* ((game->GetMap()->GetMapIndex(spritePos.x / tileSize, (spritePos.y + size - 1) / tileSize) == 3)			| (spritePos.x < 0)));
			localTouch |= (LEFT_TOP			* ((game->GetMap()->GetMapIndex(spritePos.x / tileSize, (spritePos.y + 1) / tileSize) == 3)					| (spritePos.x < 0)));

		}

		if (vertical) {

			localTouch |= (TOP_LEFT		* ((game->GetMap()->GetMapIndex((spritePos.x + 1) / tileSize, spritePos.y / tileSize) == 3)					| (spritePos.y < 0)));
			localTouch |= (TOP_RIGHT	* ((game->GetMap()->GetMapIndex((spritePos.x + size - 1) / tileSize, spritePos.y / tileSize) == 3)			| (spritePos.y < 0)));

			localTouch |= (BOTTOM_RIGHT	* ((game->GetMap()->GetMapIndex((spritePos.x + size - 1) / tileSize, (spritePos.y + size) / tileSize) == 3)	| ((spritePos.y + size + 1) > height)));
			localTouch |= (BOTTOM_LEFT	* ((game->GetMap()->GetMapIndex((spritePos.x + 1) / tileSize, (spritePos.y + size) / tileSize) == 3)		| ((spritePos.y + size + 1) > height)));

		}

	} else { // functionally-ranged off-sprite checks

		oldTouch = touch;
		touch = 0;

		if (horizontal) {
			
			touch |= (RIGHT_TOP		* ((game->GetMap()->GetMapIndex((spritePos.x + size + touchRange) / tileSize, spritePos.y / tileSize) == 3)				| ((spritePos.x + touchRange + size) > width)));
			touch |= (RIGHT_BOTTOM	* ((game->GetMap()->GetMapIndex((spritePos.x + size + touchRange) / tileSize, (spritePos.y + size) / tileSize) == 3)	| ((spritePos.x + touchRange + size) > width)));

			touch |= (LEFT_TOP		* ((game->GetMap()->GetMapIndex((spritePos.x - touchRange) / tileSize, spritePos.y / tileSize) == 3)					| ((spritePos.x - touchRange) < 0)));
			touch |= (LEFT_BOTTOM	* ((game->GetMap()->GetMapIndex((spritePos.x - touchRange) / tileSize, (spritePos.y + size) / tileSize) == 3)			| ((spritePos.x - touchRange) < 0)));

		}

		if (vertical) {
		
			touch |= (TOP_RIGHT		* ((game->GetMap()->GetMapIndex((spritePos.x + size) / tileSize, (spritePos.y - touchRange) / tileSize) == 3)			| ((spritePos.y - touchRange) < 0)));
			touch |= (TOP_LEFT		* ((game->GetMap()->GetMapIndex(spritePos.x / tileSize, (spritePos.y - touchRange) / tileSize) == 3)					| ((spritePos.y - touchRange) < 0)));
			
			touch |= (BOTTOM_RIGHT	* ((game->GetMap()->GetMapIndex((spritePos.x + size) / tileSize, (spritePos.y + size + touchRange) / tileSize) == 3)	| ((spritePos.y + touchRange + size) > height)));
			touch |= (BOTTOM_LEFT	* ((game->GetMap()->GetMapIndex(spritePos.x / tileSize, (spritePos.y + size + touchRange) / tileSize) == 3)				| ((spritePos.y + touchRange + size) > height)));

		}
	}
}

// Isolated check for overlap into non-traversable areas, and immediate sprite position correction
void Entity::CollisionCheck(bool horizontal, bool vertical) {

	int tileSize = game->GetMap()->GetTileSize();
	int x1 = spritePos.x/tileSize;
	int x2 = (spritePos.x+size)/tileSize;
	int y1 = spritePos.y/tileSize;
	int y2 = (spritePos.y+size)/tileSize;
	int width = game->GetMap()->GetWidth()*tileSize;
	int height = game->GetMap()->GetHeight()*tileSize;
	int oldX = spritePos.x;
	int oldY = spritePos.y;

	// default map-edge collision
	if (horizontal) {

		if (spritePos.x < 0)
			spritePos.x = 0;
		else if (spritePos.x > (width - size))
			spritePos.x = width - 15;
	
	}

	if (vertical) {

		if (spritePos.y < 0)
			spritePos.y = 0;
		else if (spritePos.y > (height - size))
			spritePos.y = height - size;

	}
	
	if (spritePos.x != oldX || spritePos.y != oldY )
		return;

	// check the immediate boarder of the sprite
	CheckTouch(true, horizontal, vertical);

	// straight-on wall collision
	if (horizontal) {

		if (localTouch & (RIGHT_TOP | RIGHT_BOTTOM) && moveState & MOVE_RIGHT)
			spritePos.x = x2*tileSize - (size+1);
		else if (localTouch & (LEFT_TOP | LEFT_BOTTOM) && moveState & MOVE_LEFT)
			spritePos.x = x1*tileSize + tileSize;

	}

	if (vertical) {

		if (localTouch & (TOP_RIGHT | TOP_LEFT) && moveState & MOVE_UP)
			spritePos.y = y1*tileSize + tileSize;
		else if (localTouch & (BOTTOM_RIGHT | BOTTOM_LEFT) && moveState & MOVE_DOWN)
			spritePos.y = y2*tileSize - (size+1);

	}

	if (spritePos.x != oldX || spritePos.y != oldY)
		return;

	// wall-follower AI outside turn wall alignment
	CheckTouch(false, true, true);

	if (horizontal) {

		if (oldTouch & ~touch & (BOTTOM_LEFT | TOP_LEFT) && moveState & MOVE_RIGHT)
			spritePos.x = x1*tileSize;
		else if (oldTouch & ~touch & (BOTTOM_RIGHT | TOP_RIGHT) && moveState & MOVE_LEFT)
			spritePos.x = x2*tileSize + tileSize - (size+1);

	}

	if (vertical) {

		if (oldTouch & ~touch & (LEFT_BOTTOM | RIGHT_BOTTOM) && moveState & MOVE_UP)
			spritePos.y = y2*tileSize + tileSize - (size+1);
		else if (oldTouch & ~touch & (LEFT_TOP | RIGHT_TOP) && moveState & MOVE_DOWN)
			spritePos.y = y1*tileSize;

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
	sprintf_s(buffer, "%i, %i", spritePos.x, spritePos.y);
	game->DrawOutlineText(buffer, 150, 350, 255, 255, 255);

}

// FIXME: currently assumes row and column are within array limits
// return values: 2 = currently visible, 1 = explored but in shadow, 0 = unexplored
int Entity::KnownMap(int row, int column) {

	return knownMap[row][column];
}

void Entity::AddWaypoint(int wx, int wy) {

	if (userWaypoint < 0)
		userWaypoint = 0;

	waypoints[userWaypoint].x = wx;
	waypoints[userWaypoint].y = wy;
	userWaypoint++;
	maxWaypoint++;
	
	if (userWaypoint > 2)
		userWaypoint = 0;

	if (maxWaypoint > 3)
		maxWaypoint = 3;

	if (currentWaypoint < 0 || currentWaypoint > 2)
		currentWaypoint = 0;
}

int Entity::GetCenterX() {

	return spritePos.x + (sprite->w / 2);
}

int Entity::GetCenterY() {

	return spritePos.y + (sprite->h / 2);
}

// vector math functions
int Entity::FastLength(point_s *a, point_s *b) {

	return (a->x - b->x)*(a->x - b->x) + (a->y - b->y)*(a->y - b->y);

}

void Entity::VectorNormalize(vec3_t a) {

	float length, ilength;

	length = a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
	length = SDL_sqrtf(length);		// FIXME

	if (length)
	{
		ilength = 1 / length;
		a[0] *= ilength;
		a[1] *= ilength;
		a[2] *= ilength;
	}
}

// returns the distance between two points and generates a normalized vector
float Entity::VectorNormalize2(point_s *from, point_s *to, vec3_t result) {
	
	float length, ilength;

	result[0] = (float)(to->x - from->x);
	result[1] = (float)(to->y - from->y);
	result[2] = 0.0f;		// TODO: its a 2D game for now

	length = result[0] * result[0] + result[1] * result[1] + result[2] * result[2];
	length = SDL_sqrtf(length);		// FIXME

	if (length)
	{
		ilength = 1.0f / length;
		result[0] *= ilength;
		result[1] *= ilength;
		result[2] *= ilength;
	}

	return length;
}

float Entity::DotProduct(vec3_t a, vec3_t b) {

	return (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
}

// FIXME: ensure this modifies the proper values instead of a temp variable
void Entity::VectorScale(vec3_t a, float scale, vec3_t result) {

	result[0] = a[0] * scale;
	result[1] = a[1] * scale;
	result[2] = a[2] * scale;
}

void Entity::CrossProduct(vec3_t a, vec3_t b, vec3_t result) {

	result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

//Hamiltonian of unit length quaternions
// p*q = [ (ps*qv + qs*pv + pv X qv) (ps*qs - pv.qv) ]
void Entity::QuatProduct(quat_s *p, quat_s *q, quat_s *result) {

	vec3_t one, two, three;

	VectorScale(p->vector, q->scalar, one);
	VectorScale(q->vector, p->scalar, two);
	CrossProduct(p->vector, q->vector, three);

	result->scalar = p->scalar*q->scalar - DotProduct(p->vector, q->vector);

	result->vector[0] = one[0] + two[0] + three[0];
	result->vector[1] = one[1] + two[1] + three[1];
	result->vector[2] = one[2] + two[2] + three[2];
}