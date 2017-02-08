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

	// map dimensions
	tileMap.SetCellWidth(32);
	tileMap.SetCellHeight(32);
	BuildTiles(RANDOM_MAP);
	return true;
}

//**************
// eMap::BuildTiles
// Procedure for initial map layout
// Populates a matrix for future collision and redraw
//**************
void eMap::BuildTiles(const int configuration) {
/*
// TODO: if very large random numbers are needed ( ie greater than RAND_MAX 32,767 )
#include <random>
	std::random_device r;
	std::default_random_engine engine(r());	// seed the mersenne twister
	std::uniform_int_distribution<int> uniform_dist(0, NUM_ELEMENTS);
	int r = uniform_dist(engine) % NUM_ELEMENTS;
*/
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
		eVec2 tileOrigin = eVec2((float)(row * tileMap.CellWidth()), (float)(column * tileMap.CellHeight()));
		eMath::CartesianToIsometric(tileOrigin.x, tileOrigin.y);
		tileOrigin.y -= 16;	// tileset specific offset
							// FIXME/TODO: allow for tileset master image drawing offset if its a bit wonky source
		tileOrigin.x -= 32;	// logical-to-screen isometric coordinate calculation hack
							// FIXME/BUG/TODO: make this flexibly based on the tile image sizes (image frames read in)
							// AND the cartesian size of a logical tile base

		// TODO: add one eTile per cell for now, but start layering them according to procedure/file-load
		cell.Tiles().push_back(eTile(tileOrigin, type, 0));	// DEBUG: test layer == 0
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
		BuildTiles(TRAVERSABLE_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_1))
		BuildTiles(COLLISION_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_2))
		BuildTiles(RANDOM_MAP);	

	if (input->MousePressed(SDL_BUTTON_RIGHT)) { 
		// TODO(?2/2?): funtionalize these two lines of getting mouse and camera, then converting to isometric
		// YES: make it part of the PLAYER class' input handling, and get rid of this block of code here
		eVec2 tilePoint = eVec2((float)input->GetMouseX() + game.GetCamera().GetAbsBounds().x, (float)input->GetMouseY() + game.GetCamera().GetAbsBounds().y);
		eMath::IsometricToCartesian(tilePoint.x, tilePoint.y);
		ToggleTile(tilePoint);
	}
}

//***************
// eMap::Draw
// staggered isometric draw order
// of tiles visible to the camera
//***************
void eMap::Draw() {
	// DEBUG: these constants assume a cell is square, and that its isometric projection
	// is twice as wide as it is tall, the invIsoCellHeight is halved to account for the
	// staggered isometric cell alignment
	static const float invIsoCellWidth = 1.0f / (float)(tileMap.CellWidth() << 1);
	static const float invIsoCellHeight = 1.0f / (float)(tileMap.CellHeight() >> 1);

	// FIXME: + 5 because the tiles draw staggered and not tip-to-tip so each width is halved
	// so its + 2 in both directions at least, then another + 1 to accound for any variation
	int maxHorizCells = (int)(game.GetRenderer().ViewArea().w * invIsoCellWidth) + 5;
	int maxVertCells = (int)(game.GetRenderer().ViewArea().h * invIsoCellHeight) + 5;

	eVec2 camTopLeft = eVec2(game.GetCamera().GetAbsBounds().x, game.GetCamera().GetAbsBounds().y);
	eMath::IsometricToCartesian(camTopLeft.x, camTopLeft.y);

	int startRow, startCol;
	int row, column;
	tileMap.Index(camTopLeft, startRow, startCol);		// DEBUG: this tile has isometric coordinates
	startRow -= 2;										// DEBUG: ensure enough rows cover the screen area
	row = startRow;
	column = startCol;

	// staggered tile query and draw order
	bool oddLine = false;
	for (int vertCount = 0; vertCount < maxVertCells; vertCount++) {
		for (int horizCount = 0; horizCount < maxHorizCells; horizCount++) {
			if (tileMap.IsValid(row, column)) {
				auto & cell = tileMap.Index(row, column);
				cell.Draw();
			} 
			row++; column--;
		}
		oddLine = !oddLine;
		oddLine ? startCol++ : startRow++;
		row = startRow;
		column = startCol;
	}
}



