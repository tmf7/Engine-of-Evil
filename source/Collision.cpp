#include "Game.h"

const float noCollisionFraction = 2.0f;

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
bool eCollision::ForwardCollisionTest(eCollisionModel & self, std::vector<Collision_t> & collisions) {
	static std::unordered_map<eCollisionModel *, eCollisionModel *> alreadyTested;
	static std::vector<eGridCell *> broadAreaCells;					// DEBUG(performance): static to reduce dynamic allocations
	eBounds broadPhaseBounds = GetBroadPhaseBounds(self);

	GetAreaCells(broadPhaseBounds, broadAreaCells);
	alreadyTested[&self] = &self;	// ignore self collision
	for (auto && cell : broadAreaCells) {
		if (!AABBAABBTest(broadPhaseBounds, cell->AbsBounds()))		// *will* it collide with the cell
			continue;

		for (auto && kvPair : cell->Contents()) {
			auto & collider = kvPair.second;

			// don't test the same collider twice
			if (alreadyTested.find(collider) != alreadyTested.end())
				continue;
			alreadyTested[collider] = collider;

			Collision_t collision;
			if (AABBAABBTest(broadPhaseBounds, collider->AbsBounds()) &&
				(collision = MovingAABBAABBTest(self, *collider)).owner != nullptr) {

				// FIXME/BUG(performance): populating a full list of collisions is costly
				// SOLUTION(~): only save the closest collision
				collisions.push_back(collision);
			}
		}
	}
	alreadyTested.clear();
	broadAreaCells.clear();

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
// FIXME(performance): make sure areaCells passed in avoids excessive dynamic allocation by using std::vector::reserve
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
// DEBUG: includes touching
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
// eCollision::IsAABB3DInIsometricFront
// returns true if self is closer to the isometric camera
// DEBUG: includes touching in X and Y, but not Z
// DEBUG: x increases visually down-right, y increases visually down-left, z increases visually straight up the screen
// DEBUG: self defaults to behind other if no separating axis exists
// FIXME: add cases for inter-penetration
//***************
bool eCollision::IsAABB3DInIsometricFront(const eBounds3D & self, const eBounds3D & other) {
	Uint8 separatingAxis = 0;
	if (self[1][0] <= other[0][0] || self[0][0] >= other[1][0]) separatingAxis |= 1;
	if (self[1][1] <= other[0][1] || self[0][1] >= other[1][1]) separatingAxis |= 2;
	if (self[1][2] < other[0][2] || self[0][2] > other[1][2]) separatingAxis |= 4;

	// prioritize z-axis tests (z, xz, yz, xyz)
	if (separatingAxis & 4)
		return (self[0][2] > other[1][2]);

	// test remaining axes (x, y, xy)
	switch (separatingAxis) {
		case 1: return !(self[1][0] < other[1][0]);	// x
		case 2: return !(self[1][1] < other[1][1]);	// y
		case 3: return (!(self[1][0] < other[1][0])); // xy defaults to x instead of x | y
		default: return false;	// error: inter-penetrating boxes
		// TODO(~): focus on center-point positions and extents now
	}
}

//***************
// eCollision::MovingAABBAABBTest
// sets the fraction along self.velocity where
// collision first occurs with other,
// fraction > 1.0f implies no collision,
// and bundles it with the collision normal
// and a pointer to other for convenience
// DEBUG: includes touching at the very end of self.velocity (ie fraction == 1.0f)
//***************
Collision_t eCollision::MovingAABBAABBTest(eCollisionModel & self, eCollisionModel & other) {
	Collision_t hitTest;

	// started in collision
	if (AABBAABBTest(self.AbsBounds(), other.AbsBounds())) {
		hitTest.fraction = 0.0f;
		hitTest.owner = &other;
		GetCollisionNormal(self, other, hitTest);
		return hitTest;
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
			if (selfMax[i] < otherMin[i]) return hitTest; // non-intersecting and moving apart
			if (otherMax[i] < selfMin[i]) tFirst = MAX((otherMax[i] - selfMin[i]) / velocity[i], tFirst);
			if (selfMax[i] > otherMin[i]) tLast = MIN((otherMin[i] - selfMax[i]) / velocity[i], tLast);
		}
		if (velocity[i] > 0.0f) {
			if (selfMin[i] > otherMax[i]) return hitTest; // non-intersecting and moving apart
			if (selfMax[i] < otherMin[i]) tFirst = MAX((otherMin[i] - selfMax[i]) / velocity[i], tFirst);
			if (otherMax[i] > selfMin[i]) tLast = MIN((otherMax[i] - selfMin[i]) / velocity[i], tLast);
		}
		
		// generally, too far away to make contact
		if (tFirst > tLast)
			return hitTest;
	}
	hitTest.fraction = tFirst;
	hitTest.owner = &other;
	GetCollisionNormal(self, other, hitTest);
	return hitTest;		// intersecting
}

//***************
// eCollision::GetCollisionNormal
// returns a vector based on self.velocity
// and relative position to other
// FIXME: calculate velocity based on oldOrigin in the event of SetOrigin() movement
// FIXME/BUG(!): corner vertex collision causes unstable diagonal normals
// SOLUTION: if both components are abs(x) == 1, then normalize the normal
// also, ensure the CORNER normal direction is based on the geometry, not the movment
//***************
void eCollision::GetCollisionNormal(const eCollisionModel & self, const eCollisionModel & other, Collision_t & collision) {
	eVec2 selfMin = self.AbsBounds()[0];
	eVec2 selfMax = self.AbsBounds()[1];
	eVec2 otherMin = other.AbsBounds()[0];
	eVec2 otherMax = other.AbsBounds()[1];
	eVec2 velocity = self.Velocity(); 
	
	eVec2 normal = vec2_zero;

/*
	for (int i = 0; i < 2; i++) {
		if (velocity[i] < 0.0f) {
			if (otherMax[i] < selfMin[i] || other.Origin()[i] < self.Origin()[i])
				normal[i] = 1.0f;
			else
				normal[i] = -1.0f;
		} else { // velocity[i] >= 0.0f
			if (selfMax[i] < otherMin[i] || self.Origin()[i] < other.Origin()[i]) 
				normal[i] = -1.0f;
			else
				normal[i] = 1.0f;
		}
	}

	if (SDL_fabs(normal.x) > 0.0f && SDL_fabs(normal.y) > 0.0f)
		normal.Normalize();		
*/

	float xEntry = 0.0f;
	float yEntry = 0.0f;
	// find the distance between the objects on the near and far sides for both x and y
	if (velocity.x > 0.0f) 
		xEntry = otherMin.x - selfMax.x;
	else 
		xEntry = otherMax.x - selfMin.x;

	if (velocity.y > 0.0f) 
		yEntry = otherMin.y - selfMax.y;
	else 
		yEntry = otherMax.y - selfMin.y;


	// entry times on an axis (here x enters later than y) ...depending on velocity, not actual SAT...
	// IMPORTANT: if a collider is a trigger (allows overlap) then xExit, yExit, xInvExit, yInvExit matter here
	// MY VERSION := xEntry = collision.fraction * velocity.x; yEntry = collision.fraction * velocity.y
	float xFraction = collision.fraction * velocity.x;
	float yFraction = collision.fraction * velocity.y;
	if (xFraction > yFraction) {		
		if (xEntry < 0.0f) {	// separating distance along x axis (ie min to max, or max to min ...depending on velocity, not actual SAT...)
			normal.x = 1.0f;
			normal.y = 0.0f;
		} else {
			normal.x = -1.0f;
			normal.y = 0.0f;
		}
	} else {
		if (yEntry < 0.0f) {
			normal.x = 0.0f;
			normal.y = 1.0f;
		} else {
			normal.x = 0.0f;
			normal.y = -1.0f;
		}
	}
	collision.normal = normal;
}