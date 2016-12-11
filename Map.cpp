#include "Map.h"
#include "Game.h"
#include "AI.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	//load the tile file
	tileSet = game.GetImageManager().GetImage("graphics/tiles.bmp");
	if (tileSet == nullptr)
		return false;
	
	// map dimensions
	tileMap.SetCellWidth(32);
	tileMap.SetCellHeight(32);

	// FIXME: SpatialIndexGrid cell width & height should always be the same as the tileSet frame width & height
	// EG: zoom in/out scales the pixel data, so scale the cell size
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
void eMap::BuildTiles(const tileType_t type) {
	int solid;
	byte_t * tile;
	static const byte_t * tileMapEnd = &tileMap.Index(tileMap.Rows() - 1, tileMap.Columns() - 1);

/*
// TODO: if very large random numbers are needed ( ie greater than RAND_MAX 32,767 )
#include <random>
	std::random_device r;
	std::default_random_engine engine(r());	// seed the mersenne twister
	std::uniform_int_distribution<int> uniform_dist(0, NUM_ELEMENTS);
	int r = uniform_dist(engine) % NUM_ELEMENTS;
*/

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
// TODO: move the ignoreCollision functionality to a different collision detection function,
// or perhaps some sort of tileFlags return value check (eg game->Map().IsValid(point) & TRAVERSABLE_TILE)
// #define TRAVERSABLE_TILE (DOOR|STAIRS|BRICK_FLOOR) or whatever
// and rename this tileFlags_t eMap::CheckTile(const eVec2 & point, byte_t * checkTile) const;
// TODO: ALSO get rid of the references once the functionality of tile masking is separated
//**************
bool eMap::IsValid(const eVec2 & point, bool ignoreCollision) const {
	
	if	( !tileMap.IsValid(point) )
		return false;

	if ( !ignoreCollision && tileMap.Index(point) == COLLISION_TILE )
		return false;

	return true;
}

//***************
// eMap::Think
//***************
void eMap::Think() {
	eInput * input;

	input = &game.GetInput();
	if (input->KeyPressed(SDL_SCANCODE_0))
		BuildTiles(TRAVERSABLE_TILE);					// set entire map to brick
	else if (input->KeyPressed(SDL_SCANCODE_1))
		BuildTiles(COLLISION_TILE);						// set entire map to water
	else if (input->KeyPressed(SDL_SCANCODE_2))
		BuildTiles(RANDOM_TILE);						// set entire map random (the old way)

	if (input->MousePressed(SDL_BUTTON_RIGHT))
		ToggleTile(eVec2(input->GetMouseX() + game.GetCamera().GetAbsBounds().x, input->GetMouseY() + game.GetCamera().GetAbsBounds().y));
}


//***************
// eMap::Draw
// TODO: the tileSet frame dimensions should match the tileMap cell dimensions at all times in the event of zoom in/out
// TODO: post-process all areas of the screen corresponding to where entities have visited
// dim for visited and bright for entity is within its sightRange (see AI::CheckFogOfWar(...))
//***************
void eMap::Draw() const {
	eVec2 screenPoint;
	int row, column;
	int endRow, startCol, endCol;

	// maximum number of tiles to draw on the current window (max 1 boarder tile beyond in all directions)
	// TODO: allow this value to change in the event that cell size changes
	static const int maxScreenRows = (int)(game.GetRenderer().Width() / tileMap.CellWidth()) + 2;
	static const int maxScreenColumns = (int)(game.GetRenderer().Height() / tileMap.CellHeight()) + 2;
	
	// initialize the area of the tileMap to query
	tileMap.Index(game.GetCamera().GetAbsBounds(), row, column);
	startCol = column;
	endRow = row + maxScreenRows;
	endCol = column + maxScreenColumns;
	tileMap.Validate(endRow, endCol);

	while (row <= endRow) {
		tileSet->SetFrame(tileMap.Index(row, column));
		screenPoint.x = eMath::NearestFloat((float)(row		* tileMap.CellWidth())	- game.GetCamera().GetAbsBounds().x);
		screenPoint.y = eMath::NearestFloat((float)(column	* tileMap.CellHeight()) - game.GetCamera().GetAbsBounds().y);
		game.GetRenderer().DrawImage(tileSet, screenPoint);

		column++;
		if (column > endCol) {
			column = startCol;
			row++;
		}
	}
}


