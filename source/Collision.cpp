#include "Game.h"

//***************
// eCollision::OBBOBBTest
// test for a separating axis using 
// the 8 faces of both OBBs
//***************
bool eCollision::OBBOBBTest(const eBox & a, const eBox & b) {
	float ra;
	float rb;
	float R[2][2];
	float AbsR[2][2];
	static const float EPSILON = 0.015625f;	// 1/64 (because binary fractions)

	// DEBUG: all z-values of rotation matrix R are 0,
	// except z-z which would be R[2][2] if R were 3x3
	// so it is hereafter replaced with (1.0f + EPSILON)
	static const float R22 = 1.0f + EPSILON;

	const auto & aAxes = a.Axes();
	const auto & aExtents = a.Extents();
	const auto & aCenter = a.Center();

	const auto & bAxes = b.Axes();
	const auto & bExtents = b.Extents();
	const auto & bCenter = b.Center();

	// compute rotation matrix spressing b in a' coordinate frame
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			R[i][j] = a.Axes()[i] * bAxes[j];

	// compute translation vector
	// and bring it into a's coordinate frame
	eVec2 t = bCenter - aCenter;
	t.Set(t * aAxes[0], t * aAxes[1]);

	// compute common subexpressions. add in an epsilon term to
	// counteract arithmetic erros when tow edges are parallel and
	// their corss product is (near) zero
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			AbsR[i][j] = abs(R[i][j]) + EPSILON;

	// test axes a.axes[0] and a.axes[1]
	for (int i = 0; i < 2; i++) {
		ra = aExtents[i];
		rb = bExtents[0] * AbsR[i][0] + bExtents[1] * AbsR[i][1];
		if (abs(t[i]) > ra + rb) return false;
	}

	// test axes b.axes[0] and b.axes[1]
	for (int i = 0; i < 2; i++) {
		ra = aExtents[0] * AbsR[0][i] + aExtents[1] * AbsR[1][i];
		rb = bExtents[i];
		if (abs(t[0] * R[0][i] + t[1] * R[1][i]) > ra + rb) return false;
	}
	
	// test axis a.axes[0] X b.axes[2] (which is [0,0,1] for 2D)
	ra = aExtents[1] * R22;
	rb = bExtents[0] * AbsR[0][1] + bExtents[1] * AbsR[0][0];
	if (abs(-t[1] * R22) > ra + rb) return false;

	// test axis a.axes[1] X b.axes[2] (which is [0,0,1] for 2D)
	ra = aExtents[0] * R22;
	rb = bExtents[0] * AbsR[1][1] + bExtents[1] * AbsR[1][0];
	if (abs(t[0] * R22) > ra + rb) return false;

	// test axis a.axes[2] (which is [0,0,1] for 2D) X b.axes[0]
	ra = aExtents[0] * AbsR[1][0] + aExtents[1] * AbsR[0][0];
	rb = bExtents[1] * R22;
	if (abs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;

	// test axis a.axes[2] (which is [0,0,1] for 2D) X b.axes[1]
	ra = aExtents[0] * AbsR[1][1] + aExtents[1] * AbsR[0][1];
	rb = bExtents[0] * R22;
	if (abs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;

	// no separating axis, OBBs intersecting
	return true;
}
		
//***************
// eCollision::ForwardCollisionTest
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
			for (auto && kvPair : cell->Contents()) {
				auto & collider = kvPair.second;

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
	tileMap.Validate(startRow, startCol);
	tileMap.Validate(endRow, endCol);

	for (int row = startRow; row <= endRow; row++) {
		for (int col = startCol; col <= endCol; col++) {
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
// FIXME: calculate velocity based on oldOrigin in the event of zero physics velocity, yet instant origin movement
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
