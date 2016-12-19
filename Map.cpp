#include "Map.h"
#include "Game.h"
#include "AI.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	if (!eTileImpl::InitTileTypes("graphics/tiles.bmp", "graphics/tiles_format.def"))
		return false;
	
	// map dimensions
	tileMap.SetCellWidth(32);
	tileMap.SetCellHeight(32);

	// FIXME(?): Should eSpatialIndexGrid cell width & height always be the same as the tileSet frame width & height
	// FIXME(?): the spatial breakdown for drawing should be independent of the spatial breakdown for collision detection
	// EG: zoom in/out scales the pixel data, so scale the cell size
	tileSet->Frame().w = tileMap.CellWidth();
	tileSet->Frame().h = tileMap.CellHeight();

	BuildTiles(RANDOM_MAP);
	return true;
}

//**************
// eMap::BuildTiles
// Procedure for initial map layout
// Populates a matrix for future collision and redraw
//**************
void eMap::BuildTiles(const int configuration) {
	int row, column;
	int type;
	eTile * tile;

	static const eTile * tileMapEnd = &tileMap.Index(tileMap.Rows() - 1, tileMap.Columns() - 1);

/*
// TODO: if very large random numbers are needed ( ie greater than RAND_MAX 32,767 )
#include <random>
	std::random_device r;
	std::default_random_engine engine(r());	// seed the mersenne twister
	std::uniform_int_distribution<int> uniform_dist(0, NUM_ELEMENTS);
	int r = uniform_dist(engine) % NUM_ELEMENTS;
*/
	row = 0;
	column = 0;
//	for (auto && derp : tileMap) {		// TODO: get rid of tileMapEnd ???
	for (tile = &tileMap.Index(0, 0); tile <= tileMapEnd; tile++) {
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
		// FIXME/TODO: tileMap.CellWidth/CellHeight may not be representative of a tile's actual size
		// ALSO: the final procedural generation should be allow tiles to interlock, but also some to float free
		// this currently locks each cell-sized tile into individual cells
		// AND draws the map using that breakdown as well
		*tile = eTile( eVec2( (float)(row * tileMap.CellWidth()), (float)(column * tileMap.CellHeight()) ), type );
		column++;
		if (column >= tileMap.Columns()) {		// FIXME/BUG: may need to be columns - 1 or purely > columns
			column = 0;
			row++;
		}
	}
}
//**************
// eMap::ToggleTile
// toggles the tile type at tileMap[r][c] closest to the given point
// TODO (map editor): add functionality to drag-and-drop any-size tiles (with optional snap-to-grid)
//**************
void eMap::ToggleTile(const eVec2 & point) {
	int tileType;
	eTile * tile;
	
	if (!IsValid(point, true))
		return;

	tile = &tileMap.Index(point);
	tileType = tile->Type();
	tileType++;
	if (tileType >= eTileImpl::NumTileTypes())
		tileType = 0;
	tile->SetType(tileType);
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

	if ( !ignoreCollision && eTileImpl::IsCollidableHack(tileMap.Index(point).Type()) )
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
		BuildTiles(TRAVERSABLE_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_1))
		BuildTiles(COLLISION_MAP);
	else if (input->KeyPressed(SDL_SCANCODE_2))
		BuildTiles(RANDOM_MAP);	

	if (input->MousePressed(SDL_BUTTON_RIGHT))
		ToggleTile(eVec2(input->GetMouseX() + game.GetCamera().GetAbsBounds().x, input->GetMouseY() + game.GetCamera().GetAbsBounds().y));
}


//***************
// eMap::Draw
// TODO: the tileSet frame dimensions should match the tileMap cell dimensions at all times in the event of zoom in/out
// TODO: post-process all areas of the screen corresponding to where entities have visited
// dim for visited and bright for entity is within its sightRange (see AI::CheckFogOfWar(...))
//***************
void eMap::Draw() {
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
//		tileSet->SetFrame(tileMap.Index(row, column));
		eTile & tile = tileMap.Index(row, column);
		screenPoint.x = eMath::NearestFloat(tile.Origin().x - game.GetCamera().GetAbsBounds().x);
		screenPoint.y = eMath::NearestFloat(tile.Origin().y - game.GetCamera().GetAbsBounds().y);

		game.GetRenderer().AddToRenderQueue(screenPoint, tile.Image(), tile.zDepthHack());	// DEBUG: test zDepths of 0 and 2
//		game.GetRenderer().DrawImage(tileSet, screenPoint);

		column++;
		if (column > endCol) {
			column = startCol;
			row++;
		}
	}
}



