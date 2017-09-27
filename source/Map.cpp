#include "Map.h"
#include "Game.h"
#include "AI.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	if (!LoadMap("graphics/EvilTown.map"))
		return false;
	return true;
}

// BEGIN FREEHILL mapfile read test
//**************
// eMap::LoadMap
// Populates tileMap's matrix for future collision and redraw
// using a file
// DEBUG (.map file format):
// # first line comment\n
// numColumns numRows cellWidth cellHeight\n
// # third line comment\n
// batchSubframeFilenameForTileset.bsub\n
// # fifth line comment\n
// tileSetFilename.tls\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 1 comment\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 2 comment\n
// (repeat, note that 0 as a master-tileSet-index indicates a placeholder, ie a tileMap index to skip for that layer)
// (also note that ALL read values are reduced by 1 before loading into an eTileImpl::type here)
//**************
bool eMap::LoadMap(const char * mapFilename) {
	std::ifstream	read(mapFilename);
	// unable to find/open file
	if (!read.good()) 
		return false;

	char buffer[MAX_ESTRING_LENGTH];
	int numColumns = 0;
	int numRows = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	int numLayers = 0;
	int row = 0;
	int column = 0;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the first line comment
	read >> numColumns;
	read >> numRows;
	read >> cellWidth;
	read >> cellHeight;
	read >> numLayers;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the rest of the line
	if (!VerifyRead(read))
		return false;

	// initialize each tileMap cell absBounds for image and collisionModel cell-occupancy tests
	tileMap.SetCellSize(cellWidth, cellHeight);
	for (column = 0; column < numColumns; ++column) {
		for (row = 0; row < numRows; ++row) {
			auto & cell = tileMap.Index(row, column);
			eVec2 cellMins = eVec2((float)(row * cellWidth), (float)(column * cellHeight));
			cell.SetAbsBounds( eBounds(cellMins, cellMins + eVec2((float)cellWidth, (float)cellHeight)) );
			cell.TilesOwned().reserve(numLayers);	// BUGFIX: assures the tilesOwned vector data doesn't reallocate/move and invalidate tilesToDraw
		}
	}

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the third line comment
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	if (!game.GetImageManager().BatchLoadSubframes(buffer))
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the fifth line comment
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	if (!eTileImpl::LoadTileset(buffer))
		return false;

	row = 0;
	column = 0;
	int layer = 0;

	while (!read.eof()) {
		int tileType = INVALID_ID;

		if (read.peek() == '#') {
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			row = 0;
			column = 0;
			++layer;
		} else {
			read >> tileType;
		}
		if (!VerifyRead(read))
			return false;
		
		--tileType;			// DEBUG: .map format is easier to read with 0's instead of -1's so all values are incremented by 1 when writing it
		if (tileType > INVALID_ID) {
			auto & cell = tileMap.Index(row, column);
			auto & origin = cell.AbsBounds()[0];
			cell.AddTileOwned(eTile(&cell, origin, tileType, layer));
		}

		if (read.peek() == '\n') {
			read.ignore(1, '\n');
			row = 0;
			column++;
		} else if (read.peek() == ',') {
			read.ignore(1, ',');
			row++;
			if (row >= tileMap.Rows()) {
				row = 0;
				column++;
			}
		}

		// TODO: remove this, it's just a backup in case of an excessive .map file
		if (!tileMap.IsValid(row, column))
			return false;
	}
	read.close();
	return true;
}
// END FREEHILL mapfile read test

//**************
// eMap::BuildMap
// Populates the map's spatial matrix for future collision and redraw
// "procedurally"
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
	tileMap.SetCellSize(cellWidth, cellHeight);

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

		eVec2 tileOffset = eVec2(-32.0f, -16.0f);
		//-16.0f is a tileset specific offset
		// FIXME/TODO: allow for tileset master image drawing offset if its a bit wonky source
		// -32.0f is a logical-to-screen isometric coordinate calculation hack, 
		// because the image is still a rectangle, not a rhombus, but CartesianToIsometric shifts X coordinate to the right
		// FIXME/TODO: (-0.5f * tile image width, instead of const -32.0f because each image puts the isometric tip at the image's top midpoint)
		// FIXME/BUG/TODO: make this flexibly based on the tile image sizes (image frames read in)
		// AND the cartesian size of a logical tile base

		// TODO: add one eTile per cell for now, but start layering them according to procedure/file-load
		// FIXME/TODO: collisionModel currently aligns with the CELL exactly because tiles currently visually align with cells,
		// however make the eTile::Init absBounds be procedure/file-load based
		cell.TilesOwned().clear();
		cell.TilesOwned().push_back(eTile(&cell, tileOffset, type, 0));	// DEBUG: test layer == 0
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

	auto & tile = tileMap.Index(point).TilesOwned()[0];		// FIXME: assumes only one tile exists for toggling in a eGridCell (not always true)
	int tileType = tile.Type();

	tileType++;
	if (tileType >= eTileImpl::NumTileTypes())
		tileType = 0;
//	tile.SetType(tileType);		// FIXME: the origin passed in should be the non-adjusted ortho-2D coordinates (not visual iso coords of renderImage.origin)
}

//**************
// eMap::GetMouseWorldPosition
//**************
eVec2 eMap::GetMouseWorldPosition() const {
	auto & input = game.GetInput();
	eVec2 mouseWorldPoint = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());
	mouseWorldPoint += game.GetCamera().CollisionModel().AbsBounds()[0];
	eMath::IsometricToCartesian(mouseWorldPoint.x, mouseWorldPoint.y);
	return mouseWorldPoint;
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

	if ( !ignoreCollision ) {
		for (auto & tile : tileMap.Index(point).TilesOwned()) {
			if (eTileImpl::IsCollidableHack(tile.Type()))
				return false;
		}
	}

	return true;
}

//***************
// eMap::Think
// TODO: these commands should belong to a UserCommand interface
// independent of eMap (and eMap shouldn't really have a ::Think())
//***************
void eMap::Think() {
	auto & input = game.GetInput();
	if (input.KeyPressed(SDL_SCANCODE_0))
		BuildMap(TRAVERSABLE_MAP);
	else if (input.KeyPressed(SDL_SCANCODE_1))
		BuildMap(COLLISION_MAP);
	else if (input.KeyPressed(SDL_SCANCODE_2))
		BuildMap(RANDOM_MAP);	

	if (input.MousePressed(SDL_BUTTON_RIGHT))
		ToggleTile(GetMouseWorldPosition());
}

//***************
// eMap::Draw
//***************
void eMap::Draw() {
	auto & cameraCollider = game.GetCamera().CollisionModel();

	// only redraw the map if the camera has moved, or its the start of the game
	// FIXME: change this logic when animated tiles are coded
	if (cameraCollider.GetOriginDelta().LengthSquared() > FLT_EPSILON || game.GetGameTime() < 5000) {
		visibleCells.clear();

		// use the corner cells of the camera to designate the draw area
		auto & camBounds = cameraCollider.AbsBounds();
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
					visibleCells.push_back({row, column});
				} 
				row++; column--;
			}
			oddLine = !oddLine;
			oddLine ? startCol++ : startRow++;
			oddLine ? endCol++ : endRow++;
			row = startRow;
			column = startCol;
		}
	} else {
		for (auto & visibleCell : visibleCells) {
			auto & cell = tileMap.Index(visibleCell.first, visibleCell.second);
			cell.Draw();
		}
	}
}

//***************
// eMap::DebugDraw
//***************
void eMap::DebugDraw() { 
	for (auto & visibleCell : visibleCells) {
		auto & cell = tileMap.Index(visibleCell.first, visibleCell.second);
		cell.DebugDraw();
	}
}
