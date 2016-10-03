#include "Game.h"

Entity::Entity() {

	speed = 10;
	size = 15;
	sightRange = 128;
	sight.w = sightRange;
	sight.h = sightRange;
	touchRange = 1;
	waypointRange = size / 2.0f;

	forward = ZERO_VEC3;
	left = ZERO_VEC3;
	right = ZERO_VEC3;
	rotationQuat.Set(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));
	atWaypoint = false;
	moving = false;
	currentWaypoint = goals.Back();
	moveState = MOVE_TO_GOAL;	// MOVE_RIGHT;
	oldTouch = 0;
	touch = 0;

	frameDelayCount = 0;

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

	SDL_FreeSurface(surface);
	surface = NULL;

	if (sprite && key) {

		Uint32 colorKey = SDL_MapRGB(sprite->format, 255, 0, 255);
		SDL_SetColorKey(sprite, SDL_TRUE, colorKey);
	}

	collisionRadius = (float)((sprite->w * sprite->w) + (sprite->h * sprite->h));
	collisionRadius = SDL_sqrtf(collisionRadius);
	collisionRadius /= 2.0f;

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
	if (game->GetMap()->GetMapIndex(0,0) < 3) {

		spritePos.x = 5;
		spritePos.y = 5;
		return;
	}

	// Expand the search
	while (!entity_placed) {

		// down from the top
		for (i = radius, j = 0; j <= radius; j++) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {
				spritePos.x = (float)(i*tileSize + 5);
				spritePos.y = (float)(j*tileSize + 5);
				entity_placed = true;
				break;
			}
		}

		// TODO: // test to ensure this loop isnt entered if (entity_placed)
		// in from the bottom of the last search
		for (i = radius - 1, j = radius; i >= 0 && !entity_placed; i--) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {

				spritePos.x = (float)(i*tileSize + 5);
				spritePos.y = (float)(j*tileSize + 5);
				entity_placed = true;
				break;
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
	destRect.x = (int)(spritePos.x) - game->GetMap()->GetCamera()->x;
	destRect.y = (int)(spritePos.y) - game->GetMap()->GetCamera()->y;

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
	SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);

// FREEHILL BEGIN DEBUG COLLISION CIRCLE
	// draws one pink pixel for each unique x,y point on the current collision circle
	eVec3 debugVector = ORIGIN_VEC3;
	eVec2 * debugWaypoint;
	float rotationAngle;
	int collisionX, collisionY;
	size_t node;
	
	rotationAngle = 0.0f;

	while (rotationAngle < 360.0f) {

		if (forward*debugVector >= 0) {

			collisionX = GetCenterX() + (int)(collisionRadius*debugVector.x) - game->GetMap()->GetCamera()->x;
			collisionY = GetCenterY() + (int)(collisionRadius*debugVector.y) - game->GetMap()->GetCamera()->y;
			DrawPixel(game->GetBuffer(), collisionX, collisionY, 255, 0, 255);
		}
		rotationAngle++;
		debugVector = rotationQuat*debugVector;
	}
// FREEHILL END DEBUG COLLISION CIRCLE

	// draw the waypoints
	if (goals.IsEmpty())
		return;

	node = 0;
	debugWaypoint = goals.Back();
	while(debugWaypoint != nullptr) {

		destRect.x = (int)(debugWaypoint->x) - game->GetMap()->GetCamera()->x;
		destRect.y = (int)(debugWaypoint->y) - game->GetMap()->GetCamera()->y;
		SDL_BlitSurface(sprite, NULL, game->GetBuffer(), &destRect);
		node++;
		debugWaypoint = goals.FromBack(node);
	}

}

// Ultimate Goal: select a pathfinding type (eg: compass, endpoint+obstacle adjust, waypoints+minipaths, wall follow, Area awareness, etc)
// Gather sensor information and decide how to move
void Entity::Move() {

	eVec2 spriteCenter;

	//CheckFogOfWar();	// also uncomment the Update() code in Map.cpp

	////////////////////////////////////////////
	//BEGIN WAYPOINT VECTOR MOVEMENT ALGORITHM//
	////////////////////////////////////////////

	// don't move without an objective
	if (currentWaypoint == nullptr)
		return;

	CheckLineOfSight();

	spritePos.x += (int)(speed*forward.x);
	spritePos.y += (int)(speed*forward.y);

	spriteCenter.x = (float)GetCenterX();
	spriteCenter.y = (float)GetCenterY();

	if ( !atWaypoint && moving && spriteCenter.Compare(*currentWaypoint, waypointRange) ) {	// use the waypointRange to snap?

		spritePos.Set(currentWaypoint->x, currentWaypoint->y);
		StopMoving();
		RemoveWaypoint();
		SetNextWaypoint();
		atWaypoint = true;

	} else {

		atWaypoint = false;
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
// TODO(?): use a true spatial Trace algorithm (with an endpoint and cross-section)
//******************
void Entity::CheckLineOfSight() {

	eVec3 waypointVector;				// from the sprite to the next waypoint
	eVec3 testVector;					// tested for optimal travel decision
	eVec3 bestVector;					// the highest weight < 5
	float bestWeight;					// maximum decision weight == 5 sets movementVector = vector
										// retains highest weight < 5 in the event of a full 360 degree sweep without a 5
	float distToWaypoint;				// **currently not fully utilized for checks, but a potentially useful metric**
	float rotationAngle;				// amount to rotate the testVector away from the waypointVector (CW or CCW)
	eVec2 testSpritePos;
	eVec2 testSpriteCenter;
	eVec2 lastGoodCenter;
	eVec2 testPoint;
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
	testSpriteCenter.x = (float)GetCenterX();
	testSpriteCenter.y = (float)GetCenterY();

	testPoint = *currentWaypoint;
	testPoint -= testSpriteCenter;
	distToWaypoint = testPoint.Normalize();
	waypointVector.Set(testPoint.x, testPoint.y, 0.0f);
	testVector = ORIGIN_VEC3;

	rotationAngle = 0.0f;
	bestVector = ZERO_VEC3;
	bestWeight = 0.0f;

	// collision prediction
	while (1) { // until the bestVector for the movementVector is found in a 360 degree scan

		weight = 0;

		// FIXME: simplify these operations (done too many times, just preserve the value somehow)
		testSpritePos.x = spritePos.x;
		testSpritePos.y = spritePos.y;
		testSpriteCenter.x = (float)GetCenterX();
		testSpriteCenter.y = (float)GetCenterY();
		
		while (1) {

			// TODO: add functionality to navigate out of a no-walk starting position
			
			// TODO: check if the waypoint landed somewhere in a set, or between sets (along the swept area), 
			// and immediatly set THAT as the movementVector. This avoids temporarily bypassing the waypoint for a "better" LOS

			// forward test point (starts on sprite)
			testPoint.x = testSpriteCenter.x + (int)(collisionRadius*testVector.x);
			testPoint.y = testSpriteCenter.y + (int)(collisionRadius*testVector.y);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) ||
			(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;
			
			// forward test point rotated counter-clockwise 90 degrees
			testPoint.x = testSpriteCenter.x + (int)(collisionRadius*testVector.y);
			testPoint.y = testSpriteCenter.y - (int)(collisionRadius*testVector.x);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) ||
				(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;

			// forward test point rotated clockwise 90 degrees
			testPoint.x = testSpriteCenter.x - (int)(collisionRadius*testVector.y);
			testPoint.y = testSpriteCenter.y + (int)(collisionRadius*testVector.x);

			// check for collision
			// FIXME: make this a general function of Map class
			if ((game->GetMap()->GetMapIndex(testPoint.x / tileSize, testPoint.y / tileSize) == 3) ||
				(testPoint.x > width) || (testPoint.x < 0) || (testPoint.y > height) || (testPoint.y < 0))
				break;

			weight++;

			if (weight == MAX_LOS_WEIGHT)
				break;

			// move to the next potential sprite position along the testVector
			testSpritePos.x += (int)(speed*testVector.x);
			testSpritePos.y += (int)(speed*testVector.y);
			testSpriteCenter.x = testSpritePos.x + (sprite->w / 2);
			testSpriteCenter.y = testSpritePos.y + (sprite->h / 2);
		}

		weight_mod = testVector*waypointVector;

		if ( (weight+weight_mod) > bestWeight) {

			bestWeight = weight + weight_mod;
			bestVector.Set(testVector.x, testVector.y, testVector.z);
		}

		do {
			rotationAngle += ROTATION_INCREMENT;

			if (rotationAngle >= 360) {

				if (bestWeight <= 2) {

					StopMoving();

				} else {

					forward.Set(bestVector.x, bestVector.y, bestVector.z);
					moving = true;

				}
				return;
			}

			testVector = rotationQuat*testVector;

		} while (testVector*forward < 0);	// avoid testVectors that backtrack
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
	sight.x = (int)(spritePos.x) - (sightRange / 2);
	sight.y = (int)(spritePos.y) - (sightRange / 2);

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
	int x1 = (int)(spritePos.x/tileSize);
	int x2 = (int)((spritePos.x+size)/tileSize);
	int y1 = (int)(spritePos.y/tileSize);
	int y2 = (int)((spritePos.y+size)/tileSize);
	int width = game->GetMap()->GetWidth()*tileSize;
	int height = game->GetMap()->GetHeight()*tileSize;
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

// FIXME: currently assumes row and column are within array limits
// return values: 2 = currently visible, 1 = explored but in shadow, 0 = unexplored
int Entity::GetKnownMap(int row, int column) {

	return knownMap[row][column];
}

void Entity::AddWaypoint(const eVec2 & waypoint, bool userDefined) {

	if (userDefined)
		goals.PushFront(waypoint);
	else
		trail.PushFront(waypoint);

	SetNextWaypoint();
}

int Entity::GetCenterX() {

	return (int)(spritePos.x + (sprite->w / 2));
}

int Entity::GetCenterY() {

	return (int)(spritePos.y + (sprite->h / 2));
}

void Entity::DrawPixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	if (SDL_MUSTLOCK(surface))
	{
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

void Entity::SetNextWaypoint() {

	// TODO: the ai should decide the moment, according to dead-end protocols, when to 
	// change over to MOVE_TO_TRAIL, and also set currentWaypoint = trail.Front()

	switch (moveState) {

		case MOVE_TO_GOAL: {
			currentWaypoint = goals.Back();
			break;
		}

		case MOVE_TO_TRAIL: {
			
			if (trail.IsEmpty()) {
				moveState = MOVE_TO_GOAL;
				currentWaypoint = goals.Back();
			}
			break;
		}
	}
}

void Entity::StopMoving() {
	forward.Zero();
	left.Zero();
	right.Zero();
	moving = false;
}

void Entity::RemoveWaypoint() {
	switch (moveState) {

		case MOVE_TO_GOAL: {
			goals.PopBack();
			break;
		}

		case MOVE_TO_TRAIL: {
			trail.PopFront();
			break;
		}
	}
}