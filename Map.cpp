#include "Game.h"

Map::Map() {

}

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

	BuildTiles();

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
void Map::BuildTiles() {

	int row, col;
	int solid;

	for (row = 0; row < mapRows; row++) {
		for (col = 0; col < mapCols; col++) {
			solid = rand() % 4;
			if(solid < 3)
				tileMap[row][col] = SOLID_TILE;
			else 
				tileMap[row][col] = NONSOLID_TILE;
		}
	}
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

// returns the tile type at the given index
int Map::GetIndexValue(int row, int column) const {

	if (row >= 0 && row < mapRows  && column >= 0 && column < mapCols)
		return tileMap[row][column];
	else
		return INVALID_TILE;
}

// return the tile type at the given point
int Map::GetIndexValue(const eVec2 & point) const {
	int row;
	int column;

	row = (int)(point.x / tileSize);
	column = (int)(point.y / tileSize);
	if (row >= 0 && row < mapRows  && column >= 0 && column < mapCols)
		return tileMap[row][column];
	else
		return INVALID_TILE;
}


// returns true if a sprite can walk onto the given point, false otherwise
bool Map::IsValid(const eVec2 & point) const {
	bool	validity = true;
	int		mapWidth = mapRows*tileSize;
	int		mapHeight = mapCols*tileSize;

	if ( GetIndexValue(point) < SOLID_TILE ||
		(point.x > mapWidth) || (point.x < 0) || (point.y > mapHeight) || (point.y < 0) )
		validity = false;

	return validity;
}

// sets the reference row and column to scaled, but not ranged, 
// values given a point on the tileMap
void Map::GetIndex(const eVec2 & point, int & row, int & column)  const {
	row = (int)(point.x / tileSize);
	column = (int)(point.y / tileSize);
}

// TODO: determine which tiles to use from the tileSet image
void Map::Update() {

	SDL_Rect destRect;
	SDL_Rect sourceRect;
	int i, j, startI, startJ;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond)
	int rows = (game->GetBuffer()->h / tileSize) + 2;
	int columns = (game->GetBuffer()->w / tileSize) + 2;

	// verify any user-input changes to the camera
	MoveCamera();

	startI = camera.x / tileSize;
	startJ = camera.y / tileSize;

	for (i = startI; i < startI + columns; i++) {

		for (j = startJ; j < startJ + rows; j++) {

			if (i >= 0 && i < mapRows && j >= 0 && j < mapCols ) {

				// TODO: combine the 1s/2s of entity's knownMap to fade/brighten tiles (currently either const-draws 1s OR temp-draws 2s)
				// fog-of-war cross-check for entity's area knowledge 
				if (1/*game->GetEntities()->KnownMap(i,j) == 2*/) {			
				
					sourceRect.w = tileSize;
					sourceRect.h = tileSize;
					sourceRect.y = 0;

					destRect.y = j*tileSize - camera.y;
					destRect.x = i*tileSize - camera.x;

				
					switch (tileMap[i][j]) {
						case SOLID_TILE:
							sourceRect.x = 0;
							break;
						case NONSOLID_TILE:
							sourceRect.x = tileSize;
							break;
					}
				
					SDL_BlitSurface(tileSet, &sourceRect, game->GetBuffer(), &destRect);

				}
			}
		}
	}
}

// FIXME: input is which frame/tile number from the map's tileset to get, returns an SDL_Surface* of that
SDL_Surface* Map::GetTile( int tileNumber ) {

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
		camera.x = (int)(game->GetEntities()->GetCenter().x) - game->GetBuffer()->w / 2;
		camera.y = (int)(game->GetEntities()->GetCenter().y) - game->GetBuffer()->h / 2;
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

