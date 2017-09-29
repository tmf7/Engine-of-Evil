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
	for (auto & tile : tilesOwned) {				 // FIXME: for some reason HIT/MISS only draws at certain camera positions relative to the tree's owner cell
		if (tile.Type() == 155) {	// the test tree // FIXME(!): draw any eTile that has a defined eCollisionModel (ie: non-nullptr, once moved to the heap)
			game.GetRenderer().DrawIsometricRect(pinkColor, tile.CollisionModel().AbsBounds(), true);
			auto & ai = game.GetEntity(0);
			if (eCollision::AABBAABBTest(tile.CollisionModel().AbsBounds(), ai->CollisionModel().AbsBounds()))
				game.GetRenderer().DrawOutlineText("HIT", eVec2(100.0f, 100.0f), redColor, true, RENDERTYPE_STATIC);
			else 
				game.GetRenderer().DrawOutlineText("MISS", eVec2(100.0f, 100.0f), lightBlueColor, true, RENDERTYPE_STATIC);
		}
	}
}
