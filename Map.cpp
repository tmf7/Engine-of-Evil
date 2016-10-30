#include "Map.h"
#include "Math.h"
#include "Game.h"

// FIXME: give maxRows and maxCols default values in case of invalid input
bool Map::Init (char fileName[], Game * const game, int maxRows, int maxCols) {

	if (!game)
		return false;

	this->game = game;

	if (!fileName[0])
		return false;

	//load the tile file
	tileSet = SDL_LoadBMP(fileName);

	if (!tileSet)
		return false;

	// starting view of map
	camera.position = ZERO_VEC2;
	camera.speed = 10.0f;

	// map dimensions
	tileMap.SetInvalidCell((byte_t)-1);			// largest possible unsigned char == 255
	tileMap.SetCellWidth(32);
	tileMap.SetCellHeight(32);
	tileMap.SetRowLimit(maxRows);
	tileMap.SetColumnLimit(maxCols);

	BuildTiles(RANDOM_TILE);

	return true;
}

void Map::Free() {

	if (tileSet) {
		SDL_FreeSurface(tileSet);
		tileSet = NULL;
	}
}

// Procedure for initial map layout
// Populates a matrix for future collision and redraw
void Map::BuildTiles(const int type) {
	int solid;
	byte_t * tile;
	static const byte_t * tileMapEnd = tileMap.Index(tileMap.RowLimit() - 1, tileMap.ColumnLimit() - 1);

	switch (type) {
		case RANDOM_TILE: {
			for (tile = tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				solid = rand() % 4;
				if (solid < 3)
					*tile = TRAVERSABLE_TILE;
				else
					*tile = COLLISION_TILE;
			}
			break;
		}
		case TRAVERSABLE_TILE: {
			for (tile = tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				*tile = TRAVERSABLE_TILE;
			}
			break;
		}
		case COLLISION_TILE: {
			for (tile = tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
				*tile = COLLISION_TILE;
			}
			break;
		}
		default: {	// RANDOM
			for (tile = tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
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

// toggles the tile type at tileMap[r][c] 
// closest to the given point
void Map::ToggleTile(const eVec2 & point) {
	byte_t * tile;
	
	tile = tileMap.Index(point);
	if (tile == nullptr)
		return;
	
	if (*tile == TRAVERSABLE_TILE)
		*tile = COLLISION_TILE;
	else // *tile == COLLISION_TILE
		*tile = TRAVERSABLE_TILE;

}

// returns true if a sprite can walk onto the given point, false otherwise
bool Map::IsValid(const eVec2 & point) {
	byte_t tileType;
	
	tileType = tileMap.Cell(point);
	if (tileType == COLLISION_TILE || tileType == tileMap.InvalidCell())
		return false;

	else if	( (point.x > tileMap.Width() - 1) || (point.x < 0) || (point.y > tileMap.Height() - 1 ) || (point.y < 0) )
		return false;

	return true;
}

// TODO: make this a const Draw() function
// draws the current frame
void Map::Update() {
	SDL_Rect destRect;
	SDL_Rect sourceRect;
	int i, j, startI, startJ;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond)
	static const int screenRows = (game->GetBuffer()->h / tileMap.CellWidth()) + 2;
	static const int screenColumns = (game->GetBuffer()->w / tileMap.CellHeight()) + 2;

	// FIXME: currently the spatial cell width & height == tile image width & height
	// but that shouldn't be the case in the final game
	sourceRect.w = tileMap.CellWidth();
	sourceRect.h = tileMap.CellHeight();
	sourceRect.y = 0;

	// verify any user-input changes to the camera
	// TODO: move this functionality to an input handler class
	MoveCamera();

	// NOTE: camera is never allowed to go beyond the tileMap dimensions
	tileMap.Index(camera.position, startI, startJ);
	if (startI == tileMap.InvalidIndex())
		startI = 0;
	if (startJ == tileMap.InvalidIndex())
		startJ = 0;

	// TODO: modify this "render" function to allow for blocks (or portions) of tile images within SpatialIndexGrid cells
	// to be blitted (eventually, maybe)
	for (i = startI; i < startI + screenColumns; i++) {
		for (j = startJ; j < startJ + screenRows; j++) {
			if (i >= 0 && i < tileMap.RowLimit() && j >= 0 && j < tileMap.ColumnLimit()) {

				// TODO: modulate brightness of tile if ANY entities have visited
				// and if an entity is within its sightRange to draw it bright/dim ( see Entity::CheckFogOfWar(...) ) 
				destRect.y = (j * tileMap.CellHeight()) - camera.position.y;
				destRect.x = (i * tileMap.CellWidth()) - camera.position.x;
				if (game->GetEntities()->KnownMap().Cell(i,j) == VISITED_TILE) {
					sourceRect.x = tileMap.CellWidth() * 2;	// draw it black	// NOTE: this is ONE FRAME BEHIND what the entity has
				} else {
					switch (tileMap.Cell(i, j)) {
						case TRAVERSABLE_TILE:
							sourceRect.x = 0;
							break;
						case COLLISION_TILE:
							sourceRect.x = tileMap.CellWidth();
							break;
					}
				}
				SDL_BlitSurface(tileSet, &sourceRect, game->GetBuffer(), &destRect);
			}
		}
	}
}

// TODO: make this part of a camera class instead of a member struct of Map
// Adjust the user's view within map
void Map::MoveCamera() {
	const Uint8 * keys = SDL_GetKeyboardState(NULL);
	static const int maxX = (tileMap.Width() - game->GetBuffer()->w) >= 0 ? 
							 tileMap.Width() - game->GetBuffer()->w : 0;
	static const int maxY = (tileMap.Height() - game->GetBuffer()->h) >= 0 ?
							 tileMap.Height() - game->GetBuffer()->h : 0;

	// centers the camera on the sprite
	if (keys[SDL_SCANCODE_SPACE]) {
		camera.position.x = (int)(game->GetEntities()->Center().x) - game->GetBuffer()->w / 2;
		camera.position.y = (int)(game->GetEntities()->Center().y) - game->GetBuffer()->h / 2;
	}

	// FIXME(?): change this logic to be more vector oriented???
	camera.position.y -= camera.speed * keys[SDL_SCANCODE_W] * (camera.position.y > 0);
	camera.position.y += camera.speed * keys[SDL_SCANCODE_S] * (camera.position.y < maxY);
	camera.position.x -= camera.speed * keys[SDL_SCANCODE_A] * (camera.position.x > 0);
	camera.position.x += camera.speed * keys[SDL_SCANCODE_D] * (camera.position.x < maxX);

	if (camera.position.x < 0)
		camera.position.x = 0;
	else if (camera.position.x > maxX)
		camera.position.x = maxX;

	if (camera.position.y < 0)
		camera.position.y = 0;
	else if (camera.position.y > maxY)
		camera.position.y = maxY;
}


