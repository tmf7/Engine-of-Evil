#include "Map.h"
#include "Game.h"

//**************
// eMap::Init
// TODO: allow the localBounds to be resized as the window resizes or goes fullscreen
//**************
bool eMap::Init (char filename[], eGame * const game, int maxRows, int maxCols) {
	eVec2 screenCorner;

	this->game = game;

	//load the tile file
	tileSet = game->ImageManager().GetImage(filename);
	if (tileSet == nullptr)
		return false;

	// starting view of map
	screenCorner = eVec2((float)game->Renderer().Width(), (float)game->Renderer().Height());
	camera.localBounds = eBounds(-screenCorner / 2.0f, screenCorner / 2.0f);	// variable rectangle with (0, 0) at its center)
	SetCameraOrigin(screenCorner / 2.0f);
	camera.speed = 10.0f;

	// map dimensions
	tileMap.SetCellWidth(32);
	tileMap.SetCellHeight(32);
	tileMap.SetRowLimit(maxRows);
	tileMap.SetColumnLimit(maxCols);

	// FIXME: SpatialIndexGrid cell width & height should not have to be the same
	// as the tileSet frame width & height
	tileSet->Frame()->w = tileMap.CellWidth();
	tileSet->Frame()->h = tileMap.CellHeight();

	BuildTiles(RANDOM_TILE);

	return true;
}

//**************
// eMap::BuildTiles
// Procedure for initial map layout
// Populates a matrix for future collision and redraw
//**************
void eMap::BuildTiles(const int type) {
	int solid;
	byte_t * tile;
	static const byte_t * tileMapEnd = &tileMap.Index(tileMap.RowLimit() - 1, tileMap.ColumnLimit() - 1);

	switch (type) {
		case RANDOM_TILE: {
			for (tile = &tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				solid = rand() % 4;
				if (solid < 3)
					*tile = TRAVERSABLE_TILE;
				else
					*tile = COLLISION_TILE;
			}
			break;
		}
		case TRAVERSABLE_TILE: {
			for (tile = &tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				*tile = TRAVERSABLE_TILE;
			}
			break;
		}
		case COLLISION_TILE: {
			for (tile = &tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				*tile = COLLISION_TILE;
			}
			break;
		}
		default: {	// RANDOM
			for (tile = &tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				solid = rand() % 4;
				if (solid < 3)
					*tile = TRAVERSABLE_TILE;
				else
					*tile = COLLISION_TILE;
			}
			break;
		}
	}
}
//**************
// eMap::ToggleTile
// toggles the tile type at tileMap[r][c] 
// closest to the given point
//**************
void eMap::ToggleTile(const eVec2 & point) {
	byte_t * tile;
	
	if (!IsValid(point, true))
		return;

	tile = &tileMap.Index(point);
	if (*tile == TRAVERSABLE_TILE)
		*tile = COLLISION_TILE;
	else // *tile == COLLISION_TILE
		*tile = TRAVERSABLE_TILE;

}

//**************
// eMap::IsValid
// returns true if point lies within map area
// TODO: move the ignoreCollision functionality to a different collision detection function
//**************
bool eMap::IsValid(const eVec2 & point, bool ignoreCollision) {
	
	if	( !tileMap.IsValid(point) )
		return false;

	if ( !ignoreCollision && tileMap.Index(point) == COLLISION_TILE )
		return false;

	return true;
}

//***************
// eMap::Update
// TODO: modulate brightness of tile if ANY entities have visited
// and if an entity is within its sightRange to draw it bright/dim ( see Entity::CheckFogOfWar(...) )
// TODO: modify this "render" function to allow for blocks (or portions) of tile images within SpatialIndexGrid cells
// to be blitted (eventually, maybe) given that the camera can change size (zoom out/in w/in limits)
// TODO: make this a const Draw() function
// draws the current frame
//***************
void eMap::Update() {
	eVec2 destination;
	int i, j, startI, startJ;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond)
	static const int screenRows = (int)(camera.absBounds.Width() / tileMap.CellWidth()) + 2;
	static const int screenColumns = (int)(camera.absBounds.Height() / tileMap.CellHeight()) + 2;

	// verify any user-input changes to the camera
	// TODO: move this functionality to an input handler class
	CameraInput();
	
	// initialize where to querying the tileMap for draw information 
	tileMap.Index(camera.absBounds[0], startI, startJ);

	for (i = startI; i < startI + screenRows; i++) {
		for (j = startJ; j < startJ + screenColumns; j++) {
			if (i >= 0 && i < tileMap.RowLimit() && j >= 0 && j < tileMap.ColumnLimit()) {
 
				destination.y = (float)(j * tileMap.CellHeight()) - camera.absBounds[0].y;
				destination.x = (float)(i * tileMap.CellWidth()) - camera.absBounds[0].x;

				// NOTE: this is ONE FRAME BEHIND what the entity has
				if (game->Entity(0)->KnownMap().Index(i, j) == VISITED_TILE)
					tileSet->SetFrame(RANDOM_TILE);			// draw it black
				else
					tileSet->SetFrame(tileMap.Index(i, j));	// draw it normal

				game->Renderer().DrawImage(tileSet, destination);
			}
		}
	}
}

//**************
// eMap::MoveCamera
// TODO: make this part of a camera class instead of a member struct of Map
// Adjust the user's view within map
//**************
void eMap::CameraInput() {
	eVec2 correction;
	const Uint8 * keys = SDL_GetKeyboardState(NULL);
	static const int maxX = tileMap.Width() > (int)camera.localBounds.Width() ? tileMap.Width() : (int)camera.localBounds.Width();
	static const int maxY = tileMap.Height() > (int)camera.localBounds.Height() ? tileMap.Height() : (int)camera.localBounds.Height();

	if (keys[SDL_SCANCODE_SPACE]) {
		SetCameraOrigin(game->Entity(0)->Origin());
	} else {
		camera.velocity.Set((float)(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]), (float)(keys[SDL_SCANCODE_S] - keys[SDL_SCANCODE_W]));
		UpdateCameraOrigin();
	}

	// collision response with map edge
	// TODO(?): move this to a collision detection/handling class
	correction = ZERO_VEC2;
	if (camera.absBounds[0].x < 0)
		correction.x = -camera.absBounds[0].x;
	else if (camera.absBounds[1].x > maxX)
		correction.x = maxX - camera.absBounds[1].x;

	if (camera.absBounds[0].y < 0)
		correction.y = -camera.absBounds[0].y;
	else if (camera.absBounds[1].y > maxY)
		correction.y = maxY - camera.absBounds[1].y;

	if (correction != ZERO_VEC2)
		SetCameraOrigin(camera.origin + correction);
}


