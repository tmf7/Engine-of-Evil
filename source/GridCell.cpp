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

//************
// eGridCell::DebugDraw
//************
void eGridCell::DebugDraw() {
	auto & renderer = game.GetRenderer();
	for (auto & tile : tilesOwned) {
		if (game.debugFlags.COLLISION && tile.CollisionModel() != nullptr)
			game.GetRenderer().DrawIsometricRect(pinkColor, tile.CollisionModel()->AbsBounds(), true);

		auto & renderBlock = tile.GetRenderImage()->renderBlock;
		if (game.debugFlags.RENDERBLOCKS && renderBlock.Depth() > 0)		// DEBUG(performance): currently not drawing flat renderBlocks
			game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderBlock, RENDERTYPE_DYNAMIC);
	}
}
