/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#include "Game.h"
#include "Map.h"
#include "Camera.h"

using namespace evil;

ECLASS_DEFINITION(eClass, eMap)

//**************
// eMap::eMap
//**************
eMap::eMap () {
	entities.reserve(MAX_ENTITIES);
}

//**************
// eMap::LoadMap
// Populates tileMap's matrix for future collision and redraw
// using a file
// DEBUG (.emap file format):
// # first line comment\n
// # any number of leading comments with '#'\n
// Num_Columns: numColumns\n
// Num_Rows: numRows\n
// Cell_Width: cellWidth\n
// Cell_Height: cellHeight\n
// Num_Layers: numLayers\n
// Tileset_Filename: tileSetFilename.etls\n
// Layers {\n
// layer_1_name {\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// }\n		(signifies end of a layer's definition)
// layer_2_name {\n
// master-tileSet-index, master-tileSet-index, ... master-tileSet-index\n
// }\n		(signifies end of a layer's definition)
// (repeat layer definitions for Num_Layers)
// }\n		(signifies end of ALL layers' definitions, moving on to entity map's entity definitions)
// [NOTE]: 0 as a master-tileSet-index indicates a placeholder, ie a tileMap index to skip for that layer
// [NOTE]: ALL master-tileSet-index read are reduced by 1 before loading into an eTileImpl::type
// # batch-load eEntity prefabs used on this map (defines prefabList indexes used below)\n
// # any number of leading comments with '#' between layer and entity definitions\n
// # only use [0|1] prefab batch file, to simplify entity map assignment\n
// Entity_Prefab_BatchFilename: entityPrefabBatchFilename.bprf\n
// Spawn_List_Title {\n
// prefabShortName: xPos yPos zPos	# eEntity::collisionModel::origin in orthographic 2D world-space, zPos of eEntity::renderImage::renderBlock's bottom in 3D world-space\n
// prefabShortName: xPos yPos zPos	# string float float float (everything past the last float is ignored)\n
// (repeat)
// }\n		(signifies end of the spawn list definition for this map)
//**************
bool eMap::LoadMap(const char * mapFilename) {
	std::ifstream	read(mapFilename);
	// unable to find/open file
	if (!read.good()) 
		return false;

	std::vector<eRenderImageIsometric *> sortTiles;
	char buffer[MAX_ESTRING_LENGTH];
	memset(buffer, 0, sizeof(buffer));

	int numColumns = 0;
	int numRows = 0;
	int cellWidth = 0;
	int cellHeight = 0;
	int numLayers = 0;
	int row = 0;
	int column = 0;

	while (read.peek() == '#')
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip comments at the top of the file

	for (int i = 0; i < 6; ++i) {
		SkipFileKey(read);													// value label text
		switch (i) {
			case 0: read >> numColumns; break;
			case 1: read >> numRows;	break;
			case 2: read >> cellWidth;	break;
			case 3: read >> cellHeight; break;
			case 4: read >> numLayers;	break;
			case 5: read.getline(buffer, sizeof(buffer), '\n'); break;		// tileSet filename
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip the rest of the line (BUGFIX: and the # begin layer def comment line)
		if (!VerifyRead(read))
			return false;
	}	

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
			cell.map = this;
			cell.SetGridPosition(row, column);
			eVec2 cellMins = eVec2((float)(row * cellWidth), (float)(column * cellHeight));
			cell.SetAbsBounds( eBounds(cellMins, cellMins + eVec2((float)cellWidth, (float)cellHeight)) );
			cell.TilesOwned().reserve(numLayers);	// BUGFIX: assures the tilesOwned vector data doesn't reallocate/move and invalidate tilesToDraw
		}
	}

	if (!eTileImpl::LoadTileset(buffer))
		return false;

	sortTiles.reserve(numRows * numColumns * numLayers);

	// READING LAYERS
	Uint32 layer = 0;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '{');			// ignore up past "Layers {"
	read.ignore(1, '\n');													// ignore the '\n' past '{'

	while (read.peek() != '}') {
		row = 0;
		column = 0;
		std::size_t tallestRenderBlock = 0;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '{');			// ignore up past "layer_# {"
		read.ignore(1, '\n');													// ignore the '\n' past '{'

		// read one layer
		while (read.peek() != '}') {
			int tileType = INVALID_ID;
			read >> tileType;
			if (!VerifyRead(read))
				return false;
			
			--tileType;			// DEBUG: .map format is easier to read with 0's instead of -1's so all values are incremented by 1 when writing it
			if (tileType > INVALID_ID) {
				auto & cell = tileMap.Index(row, column);
				auto & origin = cell.AbsBounds()[0];
				cell.AddTileOwned(eTile(&cell, origin, tileType, layer));

				auto tileRenderImage = cell.TilesOwned().back().GetComponent<eRenderImageIsometric>();
				if (tileRenderImage->GetRenderBlock().Depth() > tallestRenderBlock)
					tallestRenderBlock = (std::size_t)tileRenderImage->GetRenderBlock().Depth();

				sortTiles.emplace_back(tileRenderImage);
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

//			if (!tileMap.IsValid(row, column))									// TODO: remove this, it's just a backup in case of an excessive .map file
//				return false;
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// ignore layer closing brace '}\n'
		tileMap.AddLayerDepth(tallestRenderBlock);
		++layer;
	}
				
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');				// ignore layers group closing brace '}\n'
						  
	// LOADING PREFABS
	while (read.peek() == '#')
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// skip comments between the map layers and entity prefabs/spawning

	SkipFileKey(read);															// skip "Entity_Prefab_BatchFilename:"
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	game->GetEntityPrefabManager().BatchLoad(buffer);							// DEBUG: any batch errors get logged, but doesn't stop the map from loading
							  
	// SPAWNING ENTITIES
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');				// ignore "Spawn_List {\n"

	while (read.peek() != '}') {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ':');								// prefabShortName
		std::string prefabShortName(buffer);
		if (!VerifyRead(read))
			return false;

		eVec3 worldPosition;
		read >> worldPosition.x;
		read >> worldPosition.y;
		read >> worldPosition.z;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read))
			return false;

		if (!game->GetEntityPrefabManager().SpawnInstance(this, prefabShortName, worldPosition)) {
			std::string message = "Invalid prefabShortName (";
			message += prefabShortName;
			message += "), or invalid prefab file contents.";
			eErrorLogger::LogError(message.c_str(), __FILE__, __LINE__);
		}
	}

	read.close();

	// initialize the static map images sort order
	eRenderer::TopologicalDrawDepthSort(sortTiles);	
	return true;
}

//***************
// eMap::UnloadMap
// clears the current tileMap and empties the entities vector
//***************
void eMap::UnloadMap() {
	tileMap.ResetAllCells();
	ClearAllEntities();
}

//****************
// eMap::ConfigureEntity
//****************
void eMap::ConfigureEntity(int newSpawnID, const eVec3 & worldPosition, eEntity * entity) {
	entity->map = this;
	entity->SetZPosition(worldPosition.z);
	entity->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	entity->spawnedEntityID = newSpawnID;
	entity->spawnName = entity->spawnArgs.GetString("prefabShortName", TO_STRING(eEntity));
	entity->spawnName += "_" + newSpawnID;
}

//****************
// eMap::AddEntity
// finds the first unused slot in game::entities to move param entity
// and assigns it a spawnID,
// returns the new spawnID index within game::entities
// returns -1 if something went wrong
//****************
int eMap::AddEntity(std::unique_ptr<eEntity> && entity, const eVec3 & worldPosition) {
	int spawnID = 0;
	for (auto & entitySlot : entities) {
		if (entitySlot == nullptr) {
			ConfigureEntity(spawnID, worldPosition, entity.get());
			entitySlot = std::move(entity);
			return spawnID;
		} else {
			++spawnID;
		}
	}

	if (spawnID == entities.size()) {
		ConfigureEntity(spawnID, worldPosition, entity.get());
		entities.emplace_back(std::move(entity));
		return spawnID;
	}
	return -1;
}

//****************
// eMap::ClearAllEntities
//****************
void eMap::ClearAllEntities() {
	entities.clear();
}

//****************
// eMap::RemoveEntity
// DEBUG: ASSERT (entityID >= 0 && entityID < numEntities)
//****************
void eMap::RemoveEntity(int entityID) {
	entities[entityID] = nullptr;
}

//****************
// eMap::GetEntity
// DEBUG: ASSERT (entityID >= 0 && entityID < numEntities)
//****************
std::unique_ptr<eEntity> & eMap::GetEntity(int entityID) {
	return entities[entityID];
}

//****************
// eMap::NumEntities
//****************
int eMap::NumEntities() const {
	return entities.size();
}

//****************
// eMap::EntityThink
//****************
void eMap::EntityThink() {
	for (auto && entity : entities) {
		entity->UpdateComponents();
		entity->Think();
	}
}

//***************
// eMap::Draw
//***************
void eMap::Draw() {
	if (viewCamera->Moved() || game->GetGameTime() < 5000) {		// reduce visibleCells setup, except during startup
		visibleCells.clear();

/*
		// use the corner cells of the camera to designate the draw area
		// same speed in Release, several ms slower in debug
		auto & camBounds = viewCamera->AbsBounds();
		eVec2 corner = camBounds[0];
		eVec2 xAxis(camBounds[1].x, camBounds[0].y);
		eVec2 yAxis(camBounds[0].x, camBounds[1].y);
		std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
		for (auto & point : obbPoints) { eMath::IsometricToCartesian(point.x, point.y); }
		eBox cameraArea(obbPoints.data());
		eCollision::GetAreaCells(this, cameraArea, visibleCells);
*/
		// use the corner cells of the camera to designate the draw area
		const auto & camBounds = viewCamera->AbsBounds();
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
					cell.Draw(viewCamera);
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
			cell->Draw(viewCamera);
	}
}

//***************
// eMap::DebugDraw
//***************
void eMap::DebugDraw() { 
	for (auto && cell : visibleCells)
		cell->DebugDraw(viewCamera->GetDebugRenderTarget());
	
	for (auto && entity : entities)
		entity->DebugDraw(viewCamera->GetDebugRenderTarget());	
}

