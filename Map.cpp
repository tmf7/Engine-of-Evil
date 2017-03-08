#include "Map.h"
#include "Game.h"
#include "AI.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	// TODO: initialize multiple tile sheets (somewhat like sprites do)
//	if (!eTileImpl::InitTileTypes("graphics/tiles.png", "graphics/tiles_format.def"))
	if (!eTileImpl::InitTileTypes("graphics/grass_and_water.tls"))
		return false;

	BuildMap(RANDOM_MAP);
	return true;
}

//**************
// eMap::BuildMap
// Populates the map's spatial matrix for future collision and redraw
//**************
void eMap::BuildMap(const int configuration) {
/*
// TODO: if very large random numbers are needed ( ie greater than RAND_MAX 32,767 )
#include <random>
	std::random_device r;
	std::default_random_engine engine(r());	// seed the mersenne twister
	std::uniform_int_distribution<int> uniform_dist(0, NUM_ELEMENTS);
	int r = uniform_dist(engine) % NUM_ELEMENTS;
*/
	// TODO(?): make these file-loadable values
	const int cellWidth = 32;
	const int cellHeight = 32;

	tileMap.SetCellWidth(cellWidth);
	tileMap.SetCellHeight(cellHeight);

	srand(SDL_GetTicks());
	int row = 0;
	int column = 0;

	for (auto && cell : tileMap) {
		int type;
		switch (configuration) {
			case RANDOM_MAP: {
				type = rand() % eTileImpl::NumTileTypes();
				break;
			}
			case TRAVERSABLE_MAP: {
				while (eTileImpl::IsCollidableHack(type = rand() % eTileImpl::NumTileTypes()))
					;
				break;
			}
			case COLLISION_MAP: {
				while (!eTileImpl::IsCollidableHack(type = rand() % eTileImpl::NumTileTypes()))
					;
				break;
			}
			default: {	// RANDOM_MAP
				type = rand() % eTileImpl::NumTileTypes();
			break;
			}
		}

		eVec2 cellMins = eVec2((float)(row * cellWidth), (float)(column * cellHeight));
		cell.SetAbsBounds( eBounds(cellMins, cellMins + eVec2((float)cellWidth, (float)cellHeight)) );

		// tile coordinates
		eMath::CartesianToIsometric(cellMins.x, cellMins.y);
		cellMins.y -= 16;	// tileset specific offset
							// FIXME/TODO: allow for tileset master image drawing offset if its a bit wonky source
		cellMins.x -= 32;	// logical-to-screen isometric coordinate calculation hack, 
							// because the image is still a rectangle, not a rhombus, but CartesianToIsometric shifts x to the right
							// FIXME/BUG/TODO: make this flexibly based on the tile image sizes (image frames read in)
							// AND the cartesian size of a logical tile base

		// TODO: add one eTile per cell for now, but start layering them according to procedure/file-load
		// DEBUG: currently one cell-aligned tile per cell, hence the reuse of cellMins
		// FIXME: Init multiple tiles for a cell (eg: if a building is at a random location have one cell be responsible for drawing it)
		cell.Tiles().back().Init(cellMins, type, 0);	// DEBUG: test layer == 0
		column++;
		if (column >= tileMap.Columns()) {
			column = 0;
			row++;
		}
	}
}
//**************
// eMap::ToggleTile
// toggles the tile type at tileMap[r][c] closest to the given point
// TODO: stop using this, its more of a debug test for cursor location than a proper map editor
//**************
void eMap::ToggleTile(const eVec2 & point) {
	if (!IsValid(point, true))
		return;

	auto & tile = tileMap.Index(point).Tiles()[0];		// DEBUG: assumes only one tile exists for toggling in a eGridCell (not always true)
	int tileType = tile.Type();

	tileType++;
	if (tileType >= eTileImpl::NumTileTypes())
		tileType = 0;
	tile.SetType(tileType);
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

	if ( !ignoreCollision && eTileImpl::IsCollidableHack(tileMap.Index(point).Tiles()[0].Type()) )
		return false;

	return true;
}

//***************
// eMap::Think
// TODO: these commands should belong to a UserCommand interface
// independent of eMap (and eMap shouldn't really have a ::Think())
//***************
void eMap::Think() {
	eInput * input;

	input = &game.GetInput();
	if (input->KeyPressed(SDL_SCANCODE_0))
		BuildMap(TRAVERSABLE_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_1))
		BuildMap(COLLISION_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_2))
		BuildMap(RANDOM_MAP);	

	if (input->MousePressed(SDL_BUTTON_RIGHT)) { 
		// TODO(?2/2?): funtionalize these two lines of getting mouse and camera, then converting to isometric
		// YES: make it part of the PLAYER class' input handling, and get rid of this block of code here
		eVec2 tilePoint = eVec2((float)input->GetMouseX(), (float)input->GetMouseY());
		tilePoint += game.GetCamera().CollisionModel().AbsBounds()[0];
		eMath::IsometricToCartesian(tilePoint.x, tilePoint.y);
		ToggleTile(tilePoint);
	}
}

//***************
// eMap::Draw
//***************
void eMap::Draw() {
	// use the corner cells of the camera to designate the draw area
	auto & camBounds = game.GetCamera().CollisionModel().AbsBounds();
	std::array<eVec2, 4> corners;
	camBounds.ToPoints(corners.data());
	for (int i = 0; i < 4; i++)
		eMath::IsometricToCartesian(corners[i].x, corners[i].y);

	int startRow, startCol;
	int endRow, endCol;
	int finalRow, finalCol;
	tileMap.Index(corners[0], startRow, startCol);
	tileMap.Index(corners[1], endRow, endCol);
	tileMap.Index(corners[2], finalRow, finalCol);

	// DEBUG: magic numbers here based loosely on the way the camera area covers cells.
	// alternatively camBounds could be expanded by 128.0f prior to rotation, but that grabs too many cells
	// more horizontal cells
	startRow -= 2;	
	endRow += 1;	
	
	// more vertical cells
	finalCol += 3;	
	finalRow += 3;

	// staggered cell draw order
	// cells' tiles already have isometric coordinates
	int row = startRow;
	int column = startCol;
	bool oddLine = false;
	while (endRow <= finalRow || endCol <= finalCol) {	// shifting down the screen
		while(row <= endRow && column >= 0) {			// shifting across the screen
			if (tileMap.IsValid(row, column)) {
				auto & cell = tileMap.Index(row, column);
				cell.Draw();
			} 
			row++; column--;
		}
		oddLine = !oddLine;
		oddLine ? startCol++ : startRow++;
		oddLine ? endCol++ : endRow++;
		row = startRow;
		column = startCol;
	}
}



