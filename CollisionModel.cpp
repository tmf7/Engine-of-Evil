#include "CollisionModel.h"
#include "Game.h"

//***************
// eCollisionModel::ClearCellReferences
// removes the tileMap's collisionModel references 
// and removes the collisonModel's tile references
//***************
void eCollisionModel::ClearAreas() {
	for (auto && tile : areas) {
		auto & index = std::find(tile->Contents().begin(), tile->Contents().end(), this);
		if (index != tile->Contents().end())
			tile->Contents().erase(index);
	}
	areas.clear();
}

//***************
// eCollisionModel::UpdateAreas
// adds collisionModel references to the tileMap
// and adds tile references to the collisonModel
// DEBUG: called whenever the collisionModel moves
//***************
void eCollisionModel::UpdateAreas() {
	ClearAreas();

	// use absBounds' four corners to update areas
	for (int corner = 0; corner < 4; corner++) {
		eVec2 testPoint;
		switch (corner) {
			case 0: testPoint = absBounds[0]; break;
			case 1: testPoint = eVec2{ absBounds[1][0], absBounds[0][1] }; break;
			case 2: testPoint = eVec2{ absBounds[0][0] , absBounds[1][1] }; break;
			case 3: testPoint = absBounds[1]; break;
		}

		int testRow;
		int testCol;
		auto & tileMap = game.GetMap().TileMap();
		tileMap.Index(testPoint, testRow, testCol);
		if (!tileMap.IsValid(testRow, testCol))
			continue;

		// don't add the same area twice
		auto & tile = tileMap.Index(testRow, testCol);
		if (std::find(areas.begin(), areas.end(), &tile) == areas.end())
			areas.push_back(&tile);
	}

	// add the collisionModel to the tileMap
	for (auto && tile : areas) {
		tile->Contents().push_back(this);
	}
}