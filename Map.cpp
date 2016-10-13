#include "Game.h"

Map::Map() {

}

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
	camera.x = 0;
	camera.y = 0;
	camera.speed = 10;

	// map dimensions
	tileSize = 32;
	mapRows = maxRows;
	mapCols = maxCols;

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
	int * tile;

	// FIXME/BUG(?): odd, somehow changing this value larger than + (mapRows * MAX_MAP_SIZE + mapCols)
	// resets camera.speed to some very slow value (processing speed remains constant)
	static const int * tileMapEnd = (int *)(&tileMap[0][0]) + (mapRows * MAX_MAP_SIZE + mapCols);

	switch (type) {
		case RANDOM_TILE: {
			for (tile = &tileMap[0][0]; tile <= tileMapEnd; tile++) {
				solid = rand() % 4;
				if (solid < 3)
					*tile = TRAVERSABLE_TILE;
				else
					*tile = COLLISION_TILE;
			}
			break;
		}
		case TRAVERSABLE_TILE: {
			for (tile = &tileMap[0][0]; tile <= tileMapEnd; tile++) {
				*tile = TRAVERSABLE_TILE;
			}
			break;
		}
		case COLLISION_TILE: {
			for (tile = &tileMap[0][0]; tile <= tileMapEnd; tile++) {
				*tile = COLLISION_TILE;
			}
			break;
		}
		default: {	// RANDOM
			for (tile = &tileMap[0][0]; tile <= tileMapEnd; tile++) {
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
	int * tile;
	
	tile = Index(point);
	if (tile == nullptr)
		return;
	
	if (*tile == TRAVERSABLE_TILE)
		*tile = COLLISION_TILE;
	else // *tile == COLLISION_TILE
		*tile = TRAVERSABLE_TILE;

}

Map::viewport* Map::GetCamera() {
	return &camera;
}

int Map::GetRows() const {
	return mapRows;
}

int Map::GetColumns() const {
	return mapCols;
}

int Map::GetWidth() const {
	return mapRows*tileSize;
}

int Map::GetHeight() const {
	return mapCols*tileSize;
}

int Map::GetTileSize() const {
	return tileSize;
}

// sets the reference row and column to map-scaled
// values using the given point on the tileMap
// users must check for INVALID_INDEX return values
void Map::Index(const eVec2 & point, int & row, int & column)  const {
	row = (int)(point.x / tileSize);
	column = (int)(point.y / tileSize);

	if (row < 0 || row >= mapRows)
		row = INVALID_INDEX;

	if (column < 0 || column >= mapCols)
		column = INVALID_INDEX;
}

// returns a pointer to tileMap[r][c] closest to the given point
// users must check for nullptr return value
int * Map::Index(const eVec2 & point) {
	int row;
	int column;

	Index(point, row, column);
	if (row == INVALID_INDEX || column == INVALID_INDEX)
		return nullptr;
	else
		return &tileMap[row][column];
}

// return values: TRAVERSABLE_TILE, COLLISION_TILE, INVALID_TILE
int Map::IndexValue(int row, int column) const {

	if (row >= 0 && row < mapRows  && column >= 0 && column < mapCols)
		return tileMap[row][column];
	else
		return INVALID_TILE;
}

// return the tile type at the given point
// return values: TRAVERSABLE_TILE, COLLISION_TILE, INVALID_TILE
int Map::IndexValue(const eVec2 & point) {
	int * value;
	
	value = Index(point);
	if (value == nullptr)
		return INVALID_TILE;
	else
		return *value;
}


// returns true if a sprite can walk onto the given point, false otherwise
bool Map::IsValid(const eVec2 & point) {
	bool	validity = true;

	if ( IndexValue(point) != TRAVERSABLE_TILE ||
		(point.x > GetWidth()) || (point.x < 0) || (point.y > GetHeight()) || (point.y < 0) )
		validity = false;

	return validity;
}

// TODO: determine which tiles to use from the tileSet image
void Map::Update() {
	SDL_Rect destRect;
	SDL_Rect sourceRect;
	int i, j, startI, startJ;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond)
	static const int rows = (game->GetBuffer()->h / tileSize) + 2;
	static const int columns = (game->GetBuffer()->w / tileSize) + 2;

	sourceRect.w = tileSize;
	sourceRect.h = tileSize;
	sourceRect.y = 0;

	// verify any user-input changes to the camera
	MoveCamera();

	// FIXME/NOTE: camera is never allowed to go beyond the tileMap dimensions
	startI = camera.x / tileSize;
	startJ = camera.y / tileSize;

	for (i = startI; i < startI + columns; i++) {
		for (j = startJ; j < startJ + rows; j++) {
			if (i >= 0 && i < mapRows && j >= 0 && j < mapCols) {

				// TODO: modulate brightness of tile if ANY entities have visited
				// and if an entity is within its sightRange to draw it bright/dim ( see Entity::CheckFogOfWar(...) ) 
				destRect.y = (j*tileSize) - camera.y;
				destRect.x = (i*tileSize) - camera.x;
				if (game->GetEntities()->KnownMapValue(i,j) == VISITED_TILE) {
					sourceRect.x = tileSize * 2;	// draw it black	// NOTE: this is ONE FRAME BEHIND what the entity has
				} else {
					switch (tileMap[i][j]) {
						case TRAVERSABLE_TILE:
							sourceRect.x = 0;
							break;
						case COLLISION_TILE:
							sourceRect.x = tileSize;
							break;
					}
				}
				SDL_BlitSurface(tileSet, &sourceRect, game->GetBuffer(), &destRect);
			}
		}
	}
}

// TODO: input is which frame/tile number from the map's tileset to get, returns an SDL_Surface* of that
SDL_Surface * Map::GetTile( int tileNumber ) {
	return tileSet;
}

// Adjust the user's view within map
void Map::MoveCamera() {
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	int maxX;
	int maxY;

	maxX = (mapRows*tileSize) - game->GetBuffer()->w;
	maxY = (mapCols*tileSize) - game->GetBuffer()->h;

	// centers the camera on the sprite
	if (keys[SDL_SCANCODE_SPACE]) {
		camera.x = (int)(game->GetEntities()->Center().x) - game->GetBuffer()->w / 2;
		camera.y = (int)(game->GetEntities()->Center().y) - game->GetBuffer()->h / 2;
	}

	camera.y -= camera.speed * keys[SDL_SCANCODE_W] * (camera.y > 0);
	camera.y += camera.speed * keys[SDL_SCANCODE_S] * (camera.y < maxY);
	camera.x -= camera.speed * keys[SDL_SCANCODE_A] * (camera.x > 0);
	camera.x += camera.speed * keys[SDL_SCANCODE_D] * (camera.x < maxX);

	if (camera.x < 0)
		camera.x = 0;
	else if (camera.x > maxX)
		camera.x = maxX;

	if (camera.y < 0)
		camera.y = 0;
	else if (camera.y > maxY)
		camera.y = maxY;
}


