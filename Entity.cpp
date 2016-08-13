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

		x = 0;
		y = 0;
		return;
	}

	// Expand the search
	while (!entity_placed) {

		// down from the top
		for (i = radius, j = 0; j <= radius && !entity_placed; j++) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {
				x = i*tileSize;
				y = j*tileSize;
				entity_placed = true;
			}
		}

		// TODO: // test to ensure this loop isnt entered if (entity_placed)
		// in from the bottom of the last search
		for (i = radius - 1, j = radius; i >= 0 && !entity_placed; i--) {

			if (game->GetMap()->GetMapIndex(i, j) < 3) {

				x = i*tileSize;
				y = j*tileSize;
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
	destRect.x = x - game->GetMap()->GetCamera()->x;
	destRect.y = y - game->GetMap()->GetCamera()->y;

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
}

// Ultimate Goal: select a pathfinding type (eg: compass, endpoint+obstacle adjust, waypoints+minipaths, wall follow, Area awareness, etc)
// Gather sensor information and decide how to follow walls
void Entity::Move() {

	CheckSight();

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
}

//******************
// CheckSight
// FIXME: should set (other) sensor information for sightRange-based decisions (ie enemy positions, range/traversable distances, LOS, etc)
// currently just updates the knownMap
//******************
void Entity::CheckSight() {

	// highlight the subset of tiles from the tileMap that are currently visible to the entity ( ie a '2' in the array)
	// and convert the *old* tiles no longer within sight range (that had a '2') to "known" (ie a '1' in the array)
	// the 2's will be used by the map to draw bright tiles, the 1's will be grey tiles, the 0's will be black/background

	// set the fog of war properties
	sight.x = x - (sightRange / 2);
	sight.y = y - (sightRange / 2);

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

// Sets a sensor bit for every point within the entity's range in a non-traversible area
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

			localTouch |= (RIGHT_TOP		* ((game->GetMap()->GetMapIndex((x + size) / tileSize, (y + 1) / tileSize) == 3)		| ((x + size + 1) > width)));
			localTouch |= (RIGHT_BOTTOM		* ((game->GetMap()->GetMapIndex((x + size) / tileSize, (y + size - 1) / tileSize) == 3)	| ((x + size + 1) > width)));
			
			localTouch |= (LEFT_BOTTOM		* ((game->GetMap()->GetMapIndex(x / tileSize, (y + size - 1) / tileSize) == 3)			| (x < 0)));
			localTouch |= (LEFT_TOP			* ((game->GetMap()->GetMapIndex(x / tileSize, (y + 1) / tileSize) == 3)					| (x < 0)));

		}

		if (vertical) {

			localTouch |= (TOP_LEFT		* ((game->GetMap()->GetMapIndex((x + 1) / tileSize, y / tileSize) == 3)					| (y < 0)));
			localTouch |= (TOP_RIGHT	* ((game->GetMap()->GetMapIndex((x + size - 1) / tileSize, y / tileSize) == 3)			| (y < 0)));

			localTouch |= (BOTTOM_RIGHT	* ((game->GetMap()->GetMapIndex((x + size - 1) / tileSize, (y + size) / tileSize) == 3)	| ((y + size + 1) > height)));
			localTouch |= (BOTTOM_LEFT	* ((game->GetMap()->GetMapIndex((x + 1) / tileSize, (y + size) / tileSize) == 3)		| ((y + size + 1) > height)));

		}

	} else { // functionally-ranged off-sprite checks

		oldTouch = touch;
		touch = 0;

		if (horizontal) {
			
			touch |= (RIGHT_TOP		* ((game->GetMap()->GetMapIndex((x + size + touchRange) / tileSize, y / tileSize) == 3)			| ((x + touchRange + size) > width)));
			touch |= (RIGHT_BOTTOM	* ((game->GetMap()->GetMapIndex((x + size + touchRange) / tileSize, (y + size) / tileSize) == 3)	| ((x + touchRange + size) > width)));

			touch |= (LEFT_TOP		* ((game->GetMap()->GetMapIndex((x - touchRange) / tileSize, y / tileSize) == 3)					| ((x - touchRange) < 0)));
			touch |= (LEFT_BOTTOM	* ((game->GetMap()->GetMapIndex((x - touchRange) / tileSize, (y + size) / tileSize) == 3)		| ((x - touchRange) < 0)));

		}

		if (vertical) {
		
			touch |= (TOP_RIGHT		* ((game->GetMap()->GetMapIndex((x + size) / tileSize, (y - touchRange) / tileSize) == 3)		| ((y - touchRange) < 0)));
			touch |= (TOP_LEFT		* ((game->GetMap()->GetMapIndex(x / tileSize, (y - touchRange) / tileSize) == 3)					| ((y - touchRange) < 0)));
			
			touch |= (BOTTOM_RIGHT	* ((game->GetMap()->GetMapIndex((x + size) / tileSize, (y + size + touchRange) / tileSize) == 3)	| ((y + touchRange + size) > height)));
			touch |= (BOTTOM_LEFT	* ((game->GetMap()->GetMapIndex(x / tileSize, (y + size + touchRange) / tileSize) == 3)			| ((y + touchRange + size) > height)));

		}
	}
}

// Isolated check for overlap into non-traversable areas, and immediate sprite position correction
void Entity::CollisionCheck(bool horizontal, bool vertical) {

	int tileSize = game->GetMap()->GetTileSize();
	int x1 = x/tileSize;
	int x2 = (x+size)/tileSize;
	int y1 = y/tileSize;
	int y2 = (y+size)/tileSize;
	int width = game->GetMap()->GetWidth()*tileSize;
	int height = game->GetMap()->GetHeight()*tileSize;
	int oldX = x;
	int oldY = y;

	// default map-edge collision
	if (horizontal) {

		if (x < 0) 
			x = 0;
		else if (x > (width - size))
			x = width - 15;
	
	}

	if (vertical) {

		if (y < 0) 
			y = 0;
		else if (y > (height - size))
			y = height - size;

	}
	
	if ( x != oldX || y != oldY )
		return;

	// check the immediate boarder of the sprite
	CheckTouch(true, horizontal, vertical);

	// straight-on wall collision
	if (horizontal) {

		if (localTouch & (RIGHT_TOP | RIGHT_BOTTOM) && moveState & MOVE_RIGHT)
			x = x2*tileSize - (size+1);
		else if (localTouch & (LEFT_TOP | LEFT_BOTTOM) && moveState & MOVE_LEFT)
			x = x1*tileSize + tileSize;

	}

	if (vertical) {

		if (localTouch & (TOP_RIGHT | TOP_LEFT) && moveState & MOVE_UP)
			y = y1*tileSize + tileSize;
		else if (localTouch & (BOTTOM_RIGHT | BOTTOM_LEFT) && moveState & MOVE_DOWN)
			y = y2*tileSize - (size+1);

	}

	if (x != oldX || y != oldY)
		return;

	// wall-follower AI outside turn wall alignment
	CheckTouch(false, true, true);

	if (horizontal) {

		if (oldTouch & ~touch & (BOTTOM_LEFT | TOP_LEFT) && moveState & MOVE_RIGHT)
			x = x1*tileSize;
		else if (oldTouch & ~touch & (BOTTOM_RIGHT | TOP_RIGHT) && moveState & MOVE_LEFT)
			x = x2*tileSize + tileSize - (size+1);

	}

	if (vertical) {

		if (oldTouch & ~touch & (LEFT_BOTTOM | RIGHT_BOTTOM) && moveState & MOVE_UP)
			y = y2*tileSize + tileSize - (size+1);
		else if (oldTouch & ~touch & (LEFT_TOP | RIGHT_TOP) && moveState & MOVE_DOWN)
			y = y1*tileSize;

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
	sprintf_s(buffer, "%i, %i", x, y);
	game->DrawOutlineText(buffer, 150, 350, 255, 255, 255);

}

// FIXME: currently assumes row and column are within array limits
int Entity::KnownMap(int row, int column) {

	return knownMap[row][column];
}