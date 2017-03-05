#include "Game.h"
		
//***************
// ForwardCollisionTest
// proactive pre-collision check
// fills collisions vector with any of areaCells' contents
// that self collides with along its current velocity
// returns true if any collision occurs
// collisions vector is sorted from nearest to farthest
// DEBUG: collisions.size() will most often be near zero
//***************
bool eCollision::ForwardCollisionTest(eCollisionModel & self, const std::vector<eGridCell *> & areaCells, std::vector<Collision_t> & collisions) {
	static std::unordered_map<eCollisionModel *, eCollisionModel *> alreadyTested;

	eBounds broadPhaseBounds = GetBroadPhaseBounds(self);
	eCollisionModel nextSelfState(	self.Origin() + self.Velocity(), // FIXME: * game.GetFixedTime()
									self.Velocity(), 
									self.LocalBounds() );

	// only test against contents if self may collide with the cell
	for (auto && cell : areaCells) {
		if (AABBAABBTest(broadPhaseBounds, cell->AbsBounds())) {
			for (auto && collider : cell->Contents()) {

				// don't test the same collider twice
				if (alreadyTested.find(collider) != alreadyTested.end())
					continue;

				Collision_t collision;
				if (AABBAABBTest(broadPhaseBounds, collider->AbsBounds()) &&
					(collision = MovingAABBAABBTest(nextSelfState, *collider)).fraction <= 1.0f) {

					// FIXME/BUG(performance): populating a full list of collisions is costly
					// SOLUTION(~): set collision, break and return at the first collision, disregard fraction and normal
					collisions.push_back(collision);
				}
			}
		}
	}
	alreadyTested.clear();
	QuickSort(	collisions.data(), 
				collisions.size(), 
				[](auto && a, auto && b) {
					if (a.fraction < b.fraction) return -1;
					else if (a.fraction > b.fraction) return 1;
					return 0;
	});
	return !collisions.empty();
}

//***************
// eCollision::GetIsometricAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// within the given area bounds (includes touching) after
// converting the area from an AABB to an OBB (isometric bounding box)
// DEBUG: make the areaCells function-static to avoid excessive dynamic allocation
// TOOD: this will be useful for areaSelection of the isometric variety, if not eMap::Draw
//***************
void eCollision::GetIsometricAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells) {
/*	auto & tileMap = game.GetMap().TileMap();
	auto & camBounds = game.GetCamera().CollisionModel().AbsBounds();

	auto topLeft = camBounds[0];
	auto topRight = eVec2(camBounds[1].x, camBounds[0].y);
	auto bottomLeft = eVec2(camBounds[0].x, camBounds[1].y);
	auto bottomRight = camBounds[1];

	eMath::IsometricToCartesian(topLeft.x, topLeft.y);
	eMath::IsometricToCartesian(topRight.x, topRight.y);
	eMath::IsometricToCartesian(bottomLeft.x, bottomLeft.y);
	eMath::IsometricToCartesian(bottomRight.x, bottomRight.y);

	// row, column pairs
	std::array<std::pair<int, int>, 4> areaIndexes;

	tileMap.Index(topLeft, areaIndexes[0].first, areaIndexes[0].second);
	tileMap.Index(topRight, areaIndexes[1].first, areaIndexes[1].second);
	tileMap.Index(bottomLeft, areaIndexes[2].first, areaIndexes[2].second);
	tileMap.Index(bottomRight, areaIndexes[3].first, areaIndexes[3].second);

	int startRow = areaIndexes[0].first;
	int startCol = areaIndexes[0].second;
	int limitRow = areaIndexes[3].first;
	int limitCol = areaIndexes[3].second;
	int row = startRow;		
	int column = startCol;

	// staggered tile query and draw order
	bool oddLine = false;
	for (int vertCount = 0; vertCount < maxVertCells; vertCount++) {
		for (int horizCount = 0; horizCount < maxHorizCells; horizCount++) {
			if (tileMap.IsValid(row, column)) {
				areaCells.push_back(&tileMap.Index(row, column));
			}
			row++; column--;				// THIS
		}
		oddLine = !oddLine;
		oddLine ? startCol++ : startRow++;	// AND THIS are most important to navigating the OBB edge and interior
		row = startRow;
		column = startCol;
	}

////////////////////////////////////
	auto & tileMap = game.GetMap().TileMap();
	int startRow, startCol;
	int endRow, endCol;
	tileMap.Index(area[0], startRow, startCol);
	tileMap.Index(area[1], endRow, endCol);
	tileMap.Validate(startRow, startCol);
	tileMap.Validate(endRow, endCol);

	for (int row = startRow; row <= endRow; row++) {
		for (int col = startCol; col <= endCol; col++) {
			areaCells.push_back(&tileMap.Index(row, col));
		}
	}
	*/
}

//***************
// eCollision::GetAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// within the given area bounds (includes touching)
// DEBUG: make the areaCells function-static to avoid excessive dynamic allocation
//***************
void eCollision::GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	int startRow, startCol;
	int endRow, endCol;
	tileMap.Index(area[0], startRow, startCol);
	tileMap.Index(area[1], endRow, endCol);
//	tileMap.Validate(startRow, startCol);		// FIXME: either do this or the loop if-statement (this is generally faster, but may break the logic)
//	tileMap.Validate(endRow, endCol);

	for (int row = startRow; row <= endRow; row++) {
		for (int col = startCol; col <= endCol; col++) {
			if (tileMap.IsValid(row, col))
				areaCells.push_back(&tileMap.Index(row, col));
		}
	}
}

//***************
// eCollision::BroadPhaseBounds
// generates an AABB that encompasses
// the swept area of a moving eBounds
// based on the eCollisionModel velocity
//***************
eBounds eCollision::GetBroadPhaseBounds(eCollisionModel & self) {
	eBounds bpBounds;
	if (self.Velocity().x > 0.0f) {
		bpBounds[0][0] = self.AbsBounds()[0][0];
		bpBounds[1][0] = self.AbsBounds()[1][0] + self.Velocity().x;
	} else {
		bpBounds[0][0] = self.AbsBounds()[0][0] + self.Velocity().x;
		bpBounds[1][0] = self.AbsBounds()[1][0] - self.Velocity().x;
	}
		
	if (self.Velocity().y > 0.0f) {
		bpBounds[0][1] = self.AbsBounds()[0][1];
		bpBounds[1][1] = self.AbsBounds()[1][1] + self.Velocity().y;
	} else {
		bpBounds[0][1] = self.AbsBounds()[0][1] + self.Velocity().y;
		bpBounds[1][1] = self.AbsBounds()[1][1] - self.Velocity().y;
	}
	return bpBounds;
}

//***************
// eCollision::AABBContainsPoint
// returns true if the given point is within the bounds
// (includes touching)
//***************
bool eCollision::AABBContainsPoint(const eBounds & bounds, const eVec2 & point) {
	if (point.x > bounds[1].x || point.x < bounds[0].x ||
		point.y > bounds[1].y || point.y < bounds[0].y) {
		return false;
	}
	return true;
}

//***************
// eCollision::AABBAABBTest
// returns true in the case of intersection
// DEBUG: includes touching
//***************
bool eCollision::AABBAABBTest(const eBounds & self, const eBounds & other) {
	if (self[1][0] < other[0][0] || self[0][0] > other[1][0]) return false;
	if (self[1][1] < other[0][1] || self[0][1] > other[1][1]) return false;
	return true;
}

//***************
// eCollision::MovingAABBAABBTest
// sets the fraction along self.velocity where
// collision first occurs with other,
// fraction > 1.0f implies no collision,
// and bundles it with the collision normal
// and a pointer to other for convenience
// DEBUG: includes touching at the very end of self.velocity
//***************
Collision_t eCollision::MovingAABBAABBTest(eCollisionModel & self, eCollisionModel & other) {
	static const float NO_COLLISION = 2.0f;

	// started in collision
	if (AABBAABBTest(self.AbsBounds(), other.AbsBounds())) {
		return { GetCollisionNormal(self, other), 0.0f, &other };
	}

	eVec2 selfMin = self.AbsBounds()[0];
	eVec2 selfMax = self.AbsBounds()[1];
	eVec2 otherMin = other.AbsBounds()[0];
	eVec2 otherMax = other.AbsBounds()[1];
	eVec2 velocity = self.Velocity();
	float tFirst = 0.0f;
	float tLast = 1.0f;

	// determine times of first and last contact, if any
	for (int i = 0; i < 2; i++) {
		if (velocity[i] < 0.0f) {
			if (selfMax[i] < otherMin[i]) return { vec2_zero, NO_COLLISION, nullptr }; // non-intersecting and moving apart
			if (otherMax[i] < selfMin[i]) tFirst = eMath::Maximize((otherMax[i] - selfMin[i]) / velocity[i], tFirst);
			if (selfMax[i] > otherMin[i]) tLast = eMath::Minimize((otherMin[i] - selfMax[i]) / velocity[i], tLast);
		}
		if (velocity[i] > 0.0f) {
			if (selfMin[i] > otherMax[i]) return { vec2_zero, NO_COLLISION, nullptr }; // non-intersecting and moving apart
			if (selfMax[i] < otherMin[i]) tFirst = eMath::Maximize((otherMin[i] - selfMax[i]) / velocity[i], tFirst);
			if (otherMax[i] > selfMin[i]) tLast = eMath::Minimize((otherMax[i] - selfMin[i]) / velocity[i], tLast);
		}
		
		// generally, too far away to make contact
		if (tFirst > tLast)
			return { vec2_zero, NO_COLLISION, nullptr };
	}
	return { GetCollisionNormal(self, other), tFirst, &other };
}

//***************
// eCollision::GetCollisionNormal
// returns a vector based on self.velocity
// and relative position to other
//***************
eVec2 eCollision::GetCollisionNormal(eCollisionModel & self, const eCollisionModel & other) {
	eVec2 selfMin = self.AbsBounds()[0];
	eVec2 selfMax = self.AbsBounds()[1];
	eVec2 otherMin = other.AbsBounds()[0];
	eVec2 otherMax = other.AbsBounds()[1];
	eVec2 velocity = self.Velocity();

	eVec2 normal = vec2_zero;
	for (int i = 0; i < 2; i++) {
		if (velocity[i] <= 0.0f) {
			if (otherMax[i] < selfMin[i] || other.Origin()[i] < self.Origin()[i])
				normal[i] = 1.0f;
			else
				normal[i] = -1.0f;				
		}
		if (velocity[i] > 0.0f) {
			if (selfMax[i] < otherMin[i] || self.Origin()[i] < other.Origin()[i]) 
				normal[i] = -1.0f;
			else
				normal[i] = 1.0f;
		}
	}
	return normal;
}
