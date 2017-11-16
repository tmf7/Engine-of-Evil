#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() {
	auto & renderer = game.GetRenderer();
	for (auto & contentPair : renderContents)
		renderer.AddToCameraRenderPool(contentPair.second);
}

//************
// eGridCell::AddTileOwned
//************
void eGridCell::AddTileOwned(eTile && tile) {
	tilesOwned.push_back(std::move(tile));
	auto & newTile = tilesOwned.back();
	if (&(newTile.CollisionModel()) != nullptr)
		newTile.CollisionModel().SetOwner(&newTile);
}

//************
// eGridCell::DebugDraw
//************
void eGridCell::DebugDraw() {
	auto & renderer = game.GetRenderer();
	for (auto & tile : tilesOwned) {
		if (game.debugFlags.COLLISION && &tile.CollisionModel() != nullptr)
			game.GetRenderer().DrawIsometricRect(pinkColor, tile.CollisionModel().AbsBounds(), true);

		auto & renderBlock = tile.RenderImage().RenderBlock();
		if (game.debugFlags.RENDERBLOCKS && renderBlock.Depth() > 0)		// DEBUG(performance): for visual clarity, don't draw flat renderBlocks
			game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderBlock, RENDERTYPE_DYNAMIC);
	}
}
