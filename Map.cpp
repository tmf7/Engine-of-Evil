#include "Map.h"
#include "Game.h"
#include "AI.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	if (!eTileImpl::InitTileTypes("graphics/tiles.png", "graphics/tiles_format.def"))
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
		// FIXME/TODO: tileMap.CellWidth/CellHeight may not be representative of a tile's actual/visual size
		// ALSO: the final procedural generation should be allow tiles to interlock, but also some to float free
		// this currently locks each cell-sized tile into individual cells
		// AND draws the map using that breakdown as well
		eVec2 tileOrigin = eVec2((float)(row * tileMap.CellWidth()), (float)(column * tileMap.CellHeight()));
		eMath::CartesianToIsometric(tileOrigin.x, tileOrigin.y);
		tileOrigin.y -= 16;	// tileset specific offset
							// FIXME/TODO: allow for tileset master image drawing offset if its a bit wonky source
		tileOrigin.x -= 32;	// logical-to-screen isometric coordinate calculation hack
							// FIXME/BUG/TODO: make this flexibly based on the tile image sizes (image frames read in)
							// AND the cartesian size of a logical tile base
		*tile = eTile(tileOrigin, type, 0);	// DEBUG: test layer == 0
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
// TODO (map editor): add functionality to drag-and-drop any-size tiles (with optional snap-to-grid)
//**************
void eMap::ToggleTile(const eVec2 & point) {
	if (!IsValid(point, true))
		return;

	eTile & tile = tileMap.Index(point);
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

	if ( !ignoreCollision && eTileImpl::IsCollidableHack(tileMap.Index(point).Type()) )
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
		eVec2 tilePoint = eVec2((float)input->GetMouseX() + game.GetCamera().GetAbsBounds().x, (float)input->GetMouseY() + game.GetCamera().GetAbsBounds().y);
		eMath::IsometricToCartesian(tilePoint.x, tilePoint.y);
		ToggleTile(tilePoint);
	}
}

//***************
// eMap::Draw
// TODO: dim for visited and bright for entity is within its sightRange (see AI::CheckFogOfWar(...))
// TODO: repeat draw order for all other layers (algorithm currently assumes a single layer)
//***************
void eMap::Draw() {
	// DEBUG: these constants assume a cell is square, and that its isometric projection
	// is twice as wide as it is tall, the invIsoCellHeight is halved to account for the
	// staggered isometric cell alignment
	static const float invIsoCellWidth = 1.0f / (float)(tileMap.CellWidth() << 1);
	static const float invIsoCellHeight = 1.0f / (float)(tileMap.CellHeight() >> 1);

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
			if (row < tileMap.Rows() && row >= 0 && column < tileMap.Columns() && column >= 0) {
				eTile & tile = tileMap.Index(row, column);
				eVec2 screenPoint = eVec2(
					eMath::NearestFloat(tile.Origin().x - game.GetCamera().GetAbsBounds().x),
					eMath::NearestFloat(tile.Origin().y - game.GetCamera().GetAbsBounds().y)
					);
				game.GetRenderer().AddToRenderQueue(renderImage_t(screenPoint, tile.Image(), tile.Layer()));
			}
			row++; column--;
		}
		oddLine = !oddLine;
		oddLine ? startCol++ : startRow++;
		row = startRow;
		column = startCol;
	}

	// TODO: draw the screen area using a staggered approach
	// get the top left point of the visible window
	// convert from ISO to CART
	// grab the tileMap tile using that point
	// (potentially decrease the row by 1 to start at a wider point, be sure to Validate() it though)

	// query and draw tiles "diagonally" until the isometric cell width has exceeded the current window width
	// IE: from the start [r][c] go to [r++][c--]

	// once a line is complete, go back to start [r][c], trigger the ODD row bool, then start drawing from [r][c++]
	// repeat the movement across the screen width
	// once this ODD line is complete, go back to the "new" start, reset the ODD row bool, then start drawing from [r++][c]
}



