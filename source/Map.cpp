#include "Map.h"
#include "Game.h"
//#include "Movement.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	if (!LoadMap("graphics/EvilTown.map"))
		return false;
	return true;
}

//**************
// eMap::LoadMap
// Populates tileMap's matrix for future collision and redraw
// using a file
// DEBUG (.map file format):
// # first line comment\n
// numColumns numRows cellWidth cellHeight numLayers\n
// # third line comment\n
// tileSetFilename.tls\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 1 comment\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 2 comment\n
// (repeat, note that 0 as a master-tileSet-index indicates a placeholder, ie a tileMap index to skip for that layer)
// (also note that ALL read values are reduced by 1 before loading into an eTileImpl::type here)
// # end of layer n comment\n
// # batch-load eEntity prefabs used on this map (defines prefabList indexes used below)\n
// entityPrefabBatchFilename.bepf\n
// # spawn unique eEntities\n
// prefabListIndex xPos yPos zPos	# eEntity::collisionModel::origin in orthographic 2D world-space, zPos of eEntity::renderImage::renderBlock's bottom in 3D world-space\n
// prefabListIndex xPos yPos zPos	# int float float float (everything past the last float is ignored)\n
// (repeat)
//**************
bool eMap::LoadMap(const char * mapFilename) {
	std::ifstream	read(mapFilename);
	// unable to find/open file
	if (!read.good()) 
		return false;

	std::vector<renderImage_t *> sortTiles;
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
	sortTiles.reserve(numRows * numColumns * numLayers);
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

	if (!eTileImpl::LoadTileset(buffer))
		return false;

	row = 0;
	column = 0;
	int layer = 0;
	enum {
		READING_MAP,
		LOADING_PREFABS,
		SPAWNING_ENTITIES
	};

	int readState = READING_MAP;
	while (!read.eof()) {
		if (readState == READING_MAP) {
			int tileType = INVALID_ID;
			bool firstComment = false;

			if (read.peek() == '#') {
				read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				row = 0;
				column = 0;
				++layer;
				firstComment = true;
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
				const auto & tileRenderImage = cell.TilesOwned().back().GetRenderImage();
				tileRenderImage->worldClip = eBounds(tileRenderImage->origin, 
													 tileRenderImage->origin + eVec2((float)tileRenderImage->srcRect->w, (float)tileRenderImage->srcRect->h));
				sortTiles.push_back(tileRenderImage);
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

			if (firstComment && read.peek() == '#') {
				readState = LOADING_PREFABS;
				read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			} else if (!tileMap.IsValid(row, column)) {			// TODO: remove this, it's just a backup in case of an excessive .map file
				return false;
			}
		} else if (readState == LOADING_PREFABS) {
			memset(buffer, 0, sizeof(buffer));
			read.getline(buffer, sizeof(buffer), '\n');
			if (!VerifyRead(read))
				return false;

			if (!game.GetEntityPrefabManager().BatchLoadPrefabs(buffer))
				return false;

			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			readState = SPAWNING_ENTITIES;
		} else if (readState == SPAWNING_ENTITIES) {
			if (read.peek() == '#')
				break;
			
			int prefabListIndex = -1;
			eVec3 worldPosition;
			read >> prefabListIndex;
			read >> worldPosition.x;
			read >> worldPosition.y;
			read >> worldPosition.z;
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if (!VerifyRead(read))
				return false;
			
			if (!eEntity::Spawn(prefabListIndex, worldPosition)) {
				std::string message = "Invalid prefabListIndex value: " + std::to_string(prefabListIndex) + "\nOr invalid prefab file contents.";
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
			}
		}
	}
	read.close();

	// initialize the static map images sort order
	eRenderer::TopologicalDrawDepthSort(sortTiles);		// FREEHILL 3d quicksort test
	return true;
}

//**************
// eMap::ToggleTile
// toggles the tile type at tileMap[r][c] closest to the given point
// TODO: stop using this, its more of a debug test for cursor location than a proper map editor
//**************
void eMap::ToggleTile(const eVec2 & point) {

	auto & cell = tileMap.IndexValidated(point);
	if (cell.TilesOwned().empty()) {
		auto & origin = cell.AbsBounds()[0];
		cell.AddTileOwned(eTile(&cell, origin, 0, 0));	
	}

	// FIXME/BUG: assumes only one tile exists for toggling in a eGridCell (not always true)
	auto & tile = cell.TilesOwned()[0];
	int tileType = tile.Type();

	tileType++;
	if (tileType >= eTileImpl::NumTileTypes())
		tileType = 0;
	tile.SetType(tileType);
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
// eMap::HitStaticWorldHack
// returns true if point lies beyond the map area,
// or within one of the world's collider's
// FIXME: this fn is only used by eAI for probing movement (and adding waypoints), move the logic to eCollisionModel/eCollision
//**************
bool eMap::HitStaticWorldHack(const eVec2 & point) {
	if	(!tileMap.IsValid(point))
		return true;

	auto & cell = tileMap.Index(point);
	for (auto & pair : cell.Contents()) {
		if (eCollision::AABBContainsPoint(pair.second->AbsBounds(), point))
			return true;
	}
	return false;
}

//***************
// eMap::Think
// FIXME: eMap (and eMap shouldn't really have a ::Think())
//***************
void eMap::Think() {
	auto & input = game.GetInput();
	if (input.MousePressed(SDL_BUTTON_RIGHT))
		ToggleTile(GetMouseWorldPosition());
}

//***************
// eMap::Draw
//***************
void eMap::Draw() {
	auto & camera = game.GetCamera();

	// reduce re-draw calls
	// FIXME: change this logic when animated tiles are coded
	if (camera.Moved() || game.GetGameTime() < 5000) {
		visibleCells.clear();

		// use the corner cells of the camera to designate the draw area
		auto & camBounds = camera.CollisionModel().AbsBounds();
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
