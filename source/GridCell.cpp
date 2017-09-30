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
	if (!game.debugFlags.COLLISION)
		return;

	auto & renderer = game.GetRenderer();
	for (auto & tile : tilesOwned) {
		if (tile.CollisionModel() != nullptr) {	
			game.GetRenderer().DrawIsometricRect(pinkColor, tile.CollisionModel()->AbsBounds(), true);
			if (tile.Type() == 155) {
				auto & ai = game.GetEntity(0);
				// FIXME: RENDERTYPE_STATIC affects draw-order overwriting
				if (eCollision::AABBAABBTest(tile.CollisionModel()->AbsBounds(), ai->CollisionModel().AbsBounds()))
					game.GetRenderer().DrawOutlineText("HIT", eVec2(100.0f, 100.0f), redColor, true, RENDERTYPE_STATIC);
				else 
					game.GetRenderer().DrawOutlineText("MISS", eVec2(100.0f, 100.0f), lightBlueColor, true, RENDERTYPE_STATIC);
			}
		}
	}
}
