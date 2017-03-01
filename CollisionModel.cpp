#include "CollisionModel.h"
#include "Collision.h"
#include "Game.h"

//***************
// eCollisionModel::ClearCellReferences
// removes the tileMap's collisionModel references 
// and removes the collisonModel's tile references
//***************
void eCollisionModel::ClearAreas() {
	for (auto && cell : areas) {
		auto & index = std::find(cell->Contents().begin(), cell->Contents().end(), this);
		if (index != cell->Contents().end())
			cell->Contents().erase(index);
	}
	areas.clear();
}

//***************
// eCollisionModel::UpdateAreas
// adds collisionModel references to the tileMap
// and adds cell references to the collisonModel
// DEBUG: called whenever the collisionModel moves
//***************
void eCollisionModel::UpdateAreas() {
	ClearAreas();
	eCollision::GetAreaCells(absBounds, areas);
	for (auto && cell : areas) {
		cell->Contents().push_back(this);
	}
}