#include "Game.h"

//***************
// eCollisionModel::ClearCellReferences
// removes the tileMap's collisionModel references 
// and removes the collisonModel's tile references
//***************
void eCollisionModel::ClearAreas() {
	for (auto && cell : areas) {
		auto & contents = cell->Contents();
		if (contents.empty())	// FIXME: necessary to prevent a shutdown crash using std::unordered_map::find (insead of std::find)
			continue;

		auto & index = contents.find(this);
		if (index != contents.end())
			contents.erase(index);
	}
	areas.clear();
}

//***************
// eCollisionModel::UpdateAreas
// adds collisionModel references to the tileMap
// and adds cell references to the collisonModel
// DEBUG: called whenever the collisionModel moves
// FIXME(performance): this is a heavily used function and should be optimized
// cell.contents as a hashmap only helps for large collisionModels
// (however most lists are very small, eg list of 2 would be O(n=2) for both hashmap and vector 
// because hashmap would need to hash the key then compare the values)
//***************
void eCollisionModel::UpdateAreas() {
	ClearAreas();

	auto & tileMap = game.GetMap().TileMap();
	auto & cell = tileMap.IndexValidated(origin);
	if (cell.AbsBounds() != absBounds) {
		eCollision::GetAreaCells(absBounds, areas);
	} else {							// DEBUG: edge case where bounds matches its cell and winds up adding 4 areas instead of 1
		areas.push_back(&cell);
	}

	for (auto && cell : areas) {
		cell->Contents()[this] = this;
	}
}