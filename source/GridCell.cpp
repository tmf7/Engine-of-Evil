#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() {
	auto & renderer = game.GetRenderer();
	for (auto & tile : tilesToDraw)
		renderer.AddToRenderPool(tile->GetRenderImage(), RENDERTYPE_DYNAMIC);
}

//************
// eGridCell::AddTileOwned
//************
void eGridCell::AddTileOwned(eTile && tile) {
	tilesOwned.push_back(std::move(tile));
	tilesOwned.back().AssignToGrid();
}

