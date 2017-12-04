#include "Game.h"

//**************
// eMap::Init
//**************
bool eMap::Init () {
	return LoadMap("Graphics/Maps/EvilTown.emap");
}

//**************
// eMap::LoadMap
// Populates tileMap's matrix for future collision and redraw
// using a file
// DEBUG (.emap file format):
// # first line comment\n
// numColumns numRows cellWidth cellHeight numLayers\n
// # third line comment\n
// tileSetFilename.etls\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 1 comment\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// # end of layer 2 comment\n
// (repeat, note that 0 as a master-tileSet-index indicates a placeholder, ie a tileMap index to skip for that layer)
// (also note that ALL read values are reduced by 1 before loading into an eTileImpl::type here)
// # end of layer n comment\n
// # batch-load eEntity prefabs used on this map (defines prefabList indexes used below)\n
// entityPrefabBatchFilename.bprf\n
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

	std::vector<eRenderImage *> sortTiles;
	char buffer[MAX_ESTRING_LENGTH];
	int numColumns = 0;
	int numRows = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	int numLayers = 0;
	int row = 0;
	int column = 0;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the first line comment
	read >> numColumns >> numRows >> cellWidth >> cellHeight >> numLayers;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the rest of the line
	if (!VerifyRead(read))
		return false;

	tileMap.SetGridSize(numRows, numColumns);					
	tileMap.SetCellSize(cellWidth, cellHeight);

	float mapWidth = (float)tileMap.Width();
	float mapHeight = (float)tileMap.Height();
	absBounds = eBounds(vec2_zero, eVec2(mapWidth, mapHeight));
	edgeColliders = { { {eBounds(vec2_zero, eVec2(0.0f, mapHeight)),				   vec2_oneZero},	// left
						{eBounds(eVec2(mapWidth, 0.0f), eVec2(mapWidth, mapHeight)),  -vec2_oneZero},	// right
						{eBounds(vec2_zero, eVec2(mapWidth, 0.0f)),					   vec2_zeroOne},	// top
						{eBounds(eVec2(0.0f, mapHeight), eVec2(mapWidth, mapHeight)), -vec2_zeroOne} }	// bottom
	};	

	// initialize each tileMap cell absBounds for image and collisionModel cell-occupancy tests
	for (column = 0; column < numColumns; ++column) {
		for (row = 0; row < numRows; ++row) {
			auto & cell = tileMap.Index(row, column);
			cell.SetGridPosition(row, column);
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
	Uint32 layer = 0;
	enum {
		READING_MAP,
		LOADING_PREFABS,
		SPAWNING_ENTITIES
	};

	sortTiles.reserve(numRows * numColumns * numLayers);

	int readState = READING_MAP;
	size_t tallestRenderBlock = 0;
	while (!read.eof()) {
		if (readState == READING_MAP) {
			int tileType = INVALID_ID;
			bool firstComment = false;

			if (read.peek() == '#') {
				read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				tileMap.AddLayerDepth(tallestRenderBlock);
				tallestRenderBlock = 0;
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
				auto & tileRenderImage = cell.TilesOwned().back().RenderImage();
				if (tileRenderImage.GetRenderBlock().Depth() > tallestRenderBlock)
					tallestRenderBlock = (size_t)tileRenderImage.GetRenderBlock().Depth();

				sortTiles.emplace_back(&tileRenderImage);
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

			if (!game.GetEntityPrefabManager().BatchLoad(buffer))
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

			// TODO: allow users to define custom eEntity derived class Spawn functions,
			// which they can then call EngineOfEvil::RegisterSpawnFunction("functionName", FunctionPointer_t)
			// where the SpawnFunction reads a local/global spawnArgs HashMap to initialize the eEntity
			// (instead of taking a fixed-size parameter list or templated parameter pack)

			// TODO: users will derive from an EngineOfEvil eGame class and override a virtual Start() method,
			// which the base eGame::Init will call polymorphically. In the user's Start() they can
			// call RegisterSpawnFunction, or any other functions they intend to hook into the EngineOfEvil runtime
			// PROBLEM: each eEntity derived class would have to be defined...or forward declared before Start() in order to pass in the fn-pointer

			// TODO: similarly, when eGame::Run's loop hits entities->Think, the user's eEntity subclasses Think() will
			// be called polymorphically based on the runtime eEntity type provided during eGame::AddEntity(std::unique_ptr<eEntity> && entity)

			// TODO: give prefabs a spawnFunction name or classname such that eMap can find the spawnfunction by name (within an eEntityFactory.. or just static eEntity method)

			if (!game.GetEntityPrefabManager().SpawnInstance(prefabListIndex, worldPosition)) {
				std::string message = "Invalid prefabListIndex value: " + std::to_string(prefabListIndex) + "\nOr invalid prefab file contents.";
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
			}
		}
	}
	read.close();

	// initialize the static map images sort order
	eRenderer::TopologicalDrawDepthSort(sortTiles);	
	return true;
}

//***************
// eMap::Draw
/*
FIXME: [minimize draw calls]
1.) at the end of eMap::Load, call an eRenderer fn to create and draw on a mapTexture of the entire map (allocate a texture the size of the map [big big])
2.) on eMap::Draw, set the visible cells like normal
3.) on eGridCell::Draw, only add those renderImages in gridcells containing an entity (eRenderImage::isSelectable)
4.) eRenderer::FlushCameraPool should behave as normal, AND prior to the first RenderCopy, copy the mapTexture to the scalable target (moved with camera)
RESULTS: does significantly reduce draw calls, but the layering visuals are wrong; also once eMovement pathing starts the fps still drops to 166-200 from 250-500
(removed this logic)
*/
//***************
void eMap::Draw() {
	auto & camera = game.GetCamera();
	if (camera.Moved() || game.GetGameTime() < 5000) {		// reduce visibleCells setup, except during startup
		visibleCells.clear();

/*
		// use the corner cells of the camera to designate the draw area
		// same speed in Release, several ms slower in debug
		auto & camBounds = camera.CollisionModel().AbsBounds();
		eVec2 corner = camBounds[0];
		eVec2 xAxis(camBounds[1].x, camBounds[0].y);
		eVec2 yAxis(camBounds[0].x, camBounds[1].y);
		std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
		for (auto & point : obbPoints) { eMath::IsometricToCartesian(point.x, point.y); }
		eBox cameraArea(obbPoints.data());
		eCollision::GetAreaCells(cameraArea, visibleCells);
	}
*/
		// use the corner cells of the camera to designate the draw area
		const auto & camBounds = camera.CollisionModel().AbsBounds();
		std::array<eVec2, 4> corners;
		camBounds.ToPoints(corners.data());
		for (auto & point : corners) { eMath::IsometricToCartesian(point.x, point.y); }
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
					visibleCells.emplace_back(&cell);
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
		for (auto & cell : visibleCells)
			cell->Draw();
	}
}

//***************
// eMap::DebugDraw
//***************
void eMap::DebugDraw() { 
	for (auto & cell : visibleCells)
		cell->DebugDraw();
}

