
/*
global:

typedef struct sensors_s {
	bool			TOP_LEFT		: 1;
	bool			TOP_RIGHT		: 1;
	bool			RIGHT_TOP		: 1;
	bool			RIGHT_BOTTOM	: 1;
	bool			BOTTOM_RIGHT	: 1;
	bool			BOTTOM_LEFT		: 1;
	bool			LEFT_BOTTOM		: 1;
	bool			LEFT_TOP		: 1;
	void			Clear() { memset(this, 0, sizeof(*this)); }
	bool			operator==(const sensors_s & that);
	bool			operator!=(const sensors_s & that);
} sensors_t;

//***************
// sensors_s::operator==
//***************
inline bool sensors_s::operator==(const sensors_s & that) {
	return (TOP_LEFT == that.TOP_LEFT			&&
			TOP_RIGHT == that.TOP_RIGHT			&&
			RIGHT_TOP == that.RIGHT_TOP			&&
			RIGHT_BOTTOM == that.RIGHT_BOTTOM	&&
			BOTTOM_RIGHT == that.BOTTOM_RIGHT	&&
			BOTTOM_LEFT == that.BOTTOM_LEFT		&&
			LEFT_BOTTOM == that.LEFT_BOTTOM		&&
			LEFT_TOP == that.LEFT_TOP				);
}

//***************
// sensors_s::operator!=
//***************
inline bool sensors_s::operator!=(const sensors_s & that) {
	return !(*this == that);
}

typedef enum {
	MOVETYPE_NONE,
	MOVETYPE_LEFT,		// wall-follow
	MOVETYPE_RIGHT,		// wall-follow
	MOVETYPE_UP,		// wall-follow
	MOVETYPE_DOWN,		// wall-follow
	MOVETYPE_GOAL,		// waypoint tracking
	MOVETYPE_TRAIL		// waypoint tracking
} movementType_t;

eAI::private:

	struct {
		sensors_t		ranged;					// off-sprite sensors
		sensors_t		oldRanged;				// off-sprite sensors from the previous frame
		sensors_t		local;					// on-sprite sensors
		sensors_t		oldLocal;				// on-sprite sensors from the previous frame
		int				reach = 1;				// distance beyond bounding box to trigger sensors
		void			Clear() { ranged.Clear(); oldRanged.Clear(); local.Clear(); oldLocal.Clear(); }
	} touch;


// old required call order after entity origin is updated
if (pathingState == PATHTYPE_WALL) {
	eAI::CheckTouch();
	eAI::CheckCollision();
	eAI::WallFollow();
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

	// straight-on wall collision
	// correction adjusts the leading edge of collision box
	if (touch.local != touch.oldLocal) {
		correction = vec2_zero;
		knownMap.Index(oldOrigin, row, column);
		tileAbsBounds = eBounds(eVec2((float)(row * knownMap.CellWidth()), (float)(column * knownMap.CellHeight())),
			eVec2((float)(row * knownMap.CellWidth() + knownMap.CellWidth()), (float)(column * knownMap.CellHeight() + knownMap.CellHeight())));

		switch (moveState) {
			case MOVETYPE_RIGHT: {
				if (touch.local.RIGHT_TOP || touch.local.RIGHT_BOTTOM)
					correction.x = tileAbsBounds[1].x - absBounds[1].x;			// right edge of currentTile
				break;
			}
			case MOVETYPE_LEFT: {
				if (touch.local.LEFT_TOP || touch.local.LEFT_BOTTOM)
					correction.x = tileAbsBounds[0].x - absBounds[0].x;			// left edge of currentTile
				break;
			}
			case MOVETYPE_UP: {
				if (touch.local.TOP_RIGHT || touch.local.TOP_LEFT)
					correction.y = tileAbsBounds[0].y - absBounds[0].y;			// top edge of currentTile
				break;
			}
			case MOVETYPE_DOWN: {
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
			case MOVETYPE_RIGHT: {
				if ((touch.oldRanged.BOTTOM_LEFT && !touch.ranged.BOTTOM_LEFT) ||
					(touch.oldRanged.TOP_LEFT && !touch.ranged.TOP_LEFT))
					correction.x = tileAbsBounds[1].x - absBounds[0].x;			// right edge of previousTile
				break;
			}
			case MOVETYPE_LEFT: {
				if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) ||
					(touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT))
					correction.x = tileAbsBounds[0].x - absBounds[1].x;			// left edge of previousTile
				break;
			}
			case MOVETYPE_UP: {
				if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) ||
					(touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM))
					correction.y = tileAbsBounds[0].y - absBounds[1].y;			// top edge of previousTile
				break;
			}
			case MOVETYPE_DOWN: {
				if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) ||
					(touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP))
					correction.y = tileAbsBounds[1].y - absBounds[0].y;			// bottom edge of previousTile			
				break;
			}
		}
		SetOrigin(origin + correction);
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
		case MOVETYPE_RIGHT: {

			// if it has lost its wall move in that direction
			// if it hasn't lost its wall move opposite that wall
			// if it never had a wall, move down
			if ((touch.oldRanged.BOTTOM_LEFT && !touch.ranged.BOTTOM_LEFT) ||
				((touch.ranged.RIGHT_TOP || touch.ranged.RIGHT_BOTTOM) && !touch.ranged.BOTTOM_LEFT))
				moveState = MOVETYPE_DOWN;

			else if ((touch.oldRanged.TOP_LEFT && !touch.ranged.TOP_LEFT) ||
				((touch.ranged.RIGHT_TOP || touch.ranged.RIGHT_BOTTOM) && !touch.ranged.TOP_LEFT))
				moveState = MOVETYPE_UP;

			break;
		}
		case MOVETYPE_LEFT: {

			if ((touch.oldRanged.BOTTOM_RIGHT && !touch.ranged.BOTTOM_RIGHT) ||
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.BOTTOM_RIGHT))
				moveState = MOVETYPE_DOWN;

			else if ((touch.oldRanged.TOP_RIGHT && !touch.ranged.TOP_RIGHT) ||
				((touch.ranged.LEFT_TOP || touch.ranged.LEFT_BOTTOM) && !touch.ranged.TOP_RIGHT))
				moveState = MOVETYPE_UP;

			break;
		}
		case MOVETYPE_UP: {

			if ((touch.oldRanged.RIGHT_BOTTOM && !touch.ranged.RIGHT_BOTTOM) ||
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.RIGHT_BOTTOM))
				moveState = MOVETYPE_RIGHT;

			else if ((touch.oldRanged.LEFT_BOTTOM && !touch.ranged.LEFT_BOTTOM) ||
				((touch.ranged.TOP_LEFT || touch.ranged.TOP_RIGHT) && !touch.ranged.LEFT_BOTTOM))
				moveState = MOVETYPE_LEFT;

			break;
		}
		case MOVETYPE_DOWN: {

			if ((touch.oldRanged.RIGHT_TOP && !touch.ranged.RIGHT_TOP) ||
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.RIGHT_TOP))
				moveState = MOVETYPE_RIGHT;

			else if ((touch.oldRanged.LEFT_TOP && !touch.ranged.LEFT_TOP) ||
				((touch.ranged.BOTTOM_LEFT || touch.ranged.BOTTOM_RIGHT) && !touch.ranged.LEFT_TOP))
				moveState = MOVETYPE_LEFT;

			break;
		}
	}
}

//******************
// eAI::DrawTouchSensors
// debug screen printout of 8 ranged entity touch sensors' statuses
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
	sprintf_s(buffer, "%s", moveState == MOVETYPE_UP ? "UP" : moveState == MOVETYPE_DOWN ? "DOWN" : moveState == MOVETYPE_RIGHT ? "RIGHT" : moveState == MOVETYPE_LEFT ? "LEFT" : moveState == MOVETYPE_GOAL ? "GOAL" : moveState == MOVETYPE_TRAIL ? "TRAIL" : " ");
	game.GetRenderer().DrawOutlineText(buffer, eVec2(150.0f, 300.0f), 255, 255, 0);

	// entity position (center of bounding box)
	sprintf_s(buffer, "%i, %i", (int)Origin().x, (int)Origin().y);
	game.GetRenderer().DrawOutlineText(buffer, eVec2(150.0f, 350.0f), 255, 255, 255);
}
*/