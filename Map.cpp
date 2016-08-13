#include "Game.h"

Map::Map() {

}


Map::~Map() {
	
}

bool Map::Init (char fileName[], Game *const g, int xTiles, int yTiles) {

	if (!g)
		return false;

	game = g;

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

	//map dimensions
	tileSize = 32;
	width = xTiles;
	height = yTiles;

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

	int x, y;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			tileMap[x][y] = rand() % 4;
		}
	}
}

Map::viewport* Map::GetCamera() {

	return &camera;
}

int Map::GetWidth() {

	return width;
}

int Map::GetHeight() {

	return height;
}

int Map::GetTileSize() {

	return tileSize;
}

int Map::GetMapIndex(int row, int column) {

	if (row >= 0 && row < width  && column >= 0 && column < height)
		return tileMap[row][column];
	else
		return -1;
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
	CheckInput();

	startI = camera.x / tileSize;
	startJ = camera.y / tileSize;

	for (i = startI; i < startI + columns; i++) {

		for (j = startJ; j < startJ + rows; j++) {

			if (i >= 0 && i < width && j >= 0 && j < height ) {

				// TODO: combine the 1s/2s of entity's knownMap to fade/brighten tiles (currently either const-draws 1s OR temp-draws 2s)
				// fog-of-war cross-check for entity's area knowledge 
				if (game->GetEntities()->KnownMap(i,j) == 2) {			
				
					sourceRect.w = tileSize;
					sourceRect.h = tileSize;
					sourceRect.y = 0;

					destRect.y = j*tileSize - camera.y;
					destRect.x = i*tileSize - camera.x;

				
					switch (tileMap[i][j]) {
						case 0:
						case 1:
						case 2: // walk
							sourceRect.x = 0;
							break;
						case 3:	// no_walk
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

void Map::CheckInput() {

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	// Adjust the user's view within map
	camera.y -= camera.speed * keys[SDL_SCANCODE_W] * (camera.y > 0);
	camera.y += camera.speed * keys[SDL_SCANCODE_S] * (camera.y < (height*tileSize) - game->GetBuffer()->h);
	camera.x -= camera.speed * keys[SDL_SCANCODE_A] * (camera.x > 0);
	camera.x += camera.speed * keys[SDL_SCANCODE_D] * (camera.x < (width*tileSize) - game->GetBuffer()->w);

}

