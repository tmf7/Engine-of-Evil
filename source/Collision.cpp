#include "Game.h"

typedef enum {
	RIGHT	= 1,
	LEFT	= 2,
	TOP		= 4,
	BOTTOM	= 8
} eNormalDir_t;

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

	// DEBUG: all z-values of rotation matrix R are 0,
	// except z-z which would be R[2][2] if R were 3x3
	// so it is hereafter replaced with (1.0f + FLT_EPSILON)
	static const float R22 = 1.0f + FLT_EPSILON;

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
			AbsR[i][j] = abs(R[i][j]) + FLT_EPSILON;

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
// TODO: make this a proper BoxCast(absBounds, direction, length, &results)
//***************
bool eCollision::ForwardCollisionTest(const eCollisionModel & self, std::vector<Collision_t> & collisions) {
	static std::unordered_map<const eCollisionModel *, const eCollisionModel *> alreadyTested;
	static std::vector<eGridCell *> broadAreaCells;					// DEBUG(performance): static to reduce dynamic allocations
	eBounds broadPhaseBounds = GetBroadPhaseBounds(self);

	GetAreaCells(broadPhaseBounds, broadAreaCells);
	alreadyTested[&self] = &self;	// ignore self collision
	for (auto & cell : broadAreaCells) {
//		if (!AABBAABBTest(broadPhaseBounds, cell->AbsBounds()))		// will it collide with the cell
//			continue;												// DEBUG: GetAreaCells already means broadPhaseBounds collides with these cells

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
				// SOLUTION(~): only save the closest collision thats on approach (not moving away)
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
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using std::vector::reserve
//***************
void eCollision::GetAreaCells(const eBounds & area, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	int startRow, startCol;
	int endRow, endCol;
	tileMap.Index(area[0], startRow, startCol);
	tileMap.Index(area[1], endRow, endCol);
	tileMap.Validate(startRow, startCol);
	tileMap.Validate(endRow, endCol);

	for (int row = startRow; row <= endRow; ++row) {
		for (int col = startCol; col <= endCol; ++col) {
			areaCells.push_back(&tileMap.Index(row, col));
		}
	}
}

//***************
// eCollision::GetAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// along the given ray (directed line segment) (includes touching)
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using std::vector::reserve
//***************
void eCollision::GetAreaCells(const eVec2 & begin, const eVec2 dir, const float length, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	int startRow, startCol;
	int endRow, endCol;
	bool rowSwapped = false;
	bool colSwapped = false;
	tileMap.Index(begin, startRow, startCol);
	tileMap.Index(begin + dir * length, endRow, endCol);
	tileMap.Validate(startRow, startCol);
	tileMap.Validate(endRow, endCol);
	if (startRow > endRow) {
		std::swap(startRow, endRow);
		rowSwapped = true;
	}
	if (startCol > endCol) {
		std::swap(startCol, endCol);
		colSwapped = true;
	}

	Collision_t placeholderResult;
	if (XOR(rowSwapped, colSwapped)) {		// scan right to left
		for (int row = startRow; row <= endRow; ++row) {
			for (int col = endCol; col >= startCol; --col) {
				if (!RayAABBTest(begin, dir, length, tileMap.Index(row, col).AbsBounds(), placeholderResult)) {
					if (col == endCol)		// no more hits possible in this column
						--endCol;
					else					// no more hits possible in this row
						break;
				} else {
					areaCells.push_back(&tileMap.Index(row, col));
				}
			}
		}
		std::reverse(areaCells.begin(), areaCells.end());
	} else {								// scan left to right
		for (int row = startRow; row <= endRow; ++row) {
			for (int col = startCol; col <= endCol; ++col) {
				if (!RayAABBTest(begin, dir, length, tileMap.Index(row, col).AbsBounds(), placeholderResult)) {
					if (col == startCol)	// no more hits possible in this column
						++startCol;
					else					// no more hits possible in this row
						break;
				} else {
					areaCells.push_back(&tileMap.Index(row, col));
				}
			}
		} 
	}
}

//***************
// eCollision::BroadPhaseBounds
// generates an AABB that encompasses
// the swept area of a moving eBounds
// based on the eCollisionModel velocity
//***************
eBounds eCollision::GetBroadPhaseBounds(const eCollisionModel & self) {
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
	}
}

//***************
// eCollision::MovingAABBAABBTest
// returns Collision_t along self.velocity where touching first occurs with other
// DEBUG: includes touching at the very end of self.velocity (ie fraction == 1.0f)
//***************
Collision_t eCollision::MovingAABBAABBTest(const eCollisionModel & self, eCollisionModel & other) {
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
	eVec2 velocity = self.Velocity();		// DEBUG: subtract other.Velocity() for simultaneous movement
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
		
		// generally, too disjoint to make contact
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
// sets the collision's surface-normal based on point's position on bounds
// does not predict collision normal if not yet in contact
// DEBUG: this only works for AABB, not OBB or general convex polygons
// DEBUG: not touching, or overlapping beyond touching is zero normal vector
//***************
void eCollision::GetCollisionNormal(const eVec2 & point, const eBounds & bounds, Collision_t & collision) {
	Uint8 entryDir = (RIGHT * (abs(point.x - bounds[0].x) == 0.0f)) 
					| (LEFT * (abs(point.x - bounds[1].x) == 0.0f)) 
					| (TOP * (abs(point.y - bounds[0].y) == 0.0f)) 
					| (BOTTOM * (abs(point.y - bounds[1].y) == 0.0f));
	SetAABBNormal(entryDir, collision.normal);
}

//***************
// eCollision::GetCollisionNormal
// sets the collision's surface-normal based on self's relative position to other
// predicts collision normal if not yet in contact
// TODO: trigger colliders (ie: allowing traversal) may need exit fraction and normal information
// DEBUG: this only works for AABB, not OBB or general convex polygons
// DEBUG: overlapping beyond touching is zero normal vector
//***************
void eCollision::GetCollisionNormal(const eCollisionModel & self, const eCollisionModel & other, Collision_t & collision) {
	eVec2 selfMin = self.AbsBounds()[0];
	eVec2 selfMax = self.AbsBounds()[1];
	eVec2 otherMin = other.AbsBounds()[0];
	eVec2 otherMax = other.AbsBounds()[1];
	eVec2 velocity = self.Velocity(); 
	
	// distance from self to other for normal on other's surface
	float xRightEntryDist = abs(selfMin.x - otherMax.x);
	float xLeftEntryDist = abs(otherMin.x - selfMax.x);
	float yTopEntryDist = abs(selfMax.y - otherMin.y);
	float yBottomEntryDist = abs(selfMin.y - otherMax.y);

	Uint8 entryDir = 0;
	if (collision.fraction == 0.0f) {
		entryDir |= (RIGHT * (xRightEntryDist == 0.0f)) 
					| (LEFT * (xLeftEntryDist == 0.0f)) 
					| (TOP * (yTopEntryDist == 0.0f)) 
					| (BOTTOM * (yBottomEntryDist == 0.0f));
	} else {
		float xFraction = abs( velocity.x != 0.0f ? MIN(xRightEntryDist, xLeftEntryDist) / velocity.x : 0.0f);
		float yFraction = abs( velocity.y != 0.0f ? MIN(yTopEntryDist, yBottomEntryDist) / velocity.y : 0.0f);
		
		if (xFraction == yFraction) {
			entryDir |=	(xRightEntryDist < xLeftEntryDist ? RIGHT : LEFT);
			entryDir |= (yTopEntryDist < yBottomEntryDist ? TOP : BOTTOM);
		} else if (xFraction < yFraction) {
			entryDir |=	(xRightEntryDist < xLeftEntryDist ? RIGHT : LEFT);
		} else {
			entryDir |= (yTopEntryDist < yBottomEntryDist ? TOP : BOTTOM);
		}
	}

	SetAABBNormal(entryDir, collision.normal);

/*
	switch(entryDir) {
		case RIGHT: 
			normal.x = -1.0f;
			normal.y = 0.0f;
			break;
		case LEFT: 
			normal.x = 1.0f;
			normal.y = 0.0f;
			break;
		case TOP:
			normal.x = 0.0f;
			normal.y = -1.0f;
			break;
		case BOTTOM:
			normal.x = 0.0f;
			normal.y = 1.0f;
			break;
		case RIGHT | TOP:
			normal.x = -0.707f;
			normal.y = -0.707f;
			break;
		case RIGHT | BOTTOM:
			normal.x = -0.707f;
			normal.y = 0.707f;
			break;
		case LEFT | TOP:
			normal.x = 0.707f;
			normal.y = -0.707f;
			break;
		case LEFT | BOTTOM:
			normal.x = 0.707f;
			normal.y = 0.707f;
			break;
	}
	collision.normal = normal;
*/
}

//***************
// eCollision::RayAABBTest
// ray: Point = begin + t * dir
// plane: Point * normal = distFromOrigin
// returns true if the ray (directed line segment) intersects the bounds (includes touching)
// and sets the result fraction and collision normal
// DEBUG: result.fraction will be [0, length] after confirmed collision
// DEBUG: dir must be unit length
//***************
bool eCollision::RayAABBTest(const eVec2 & begin, const eVec2 & dir, const float length, const eBounds & bounds, Collision_t & result) {
	float tFirst = 0.0f;
	float tLast = FLT_MAX;

	for (int i = 0; i < 2; ++i) {
		if (abs(dir[i]) < FLT_EPSILON) {		// parallel to both planes of this slab
			if (begin[i] < bounds[0][i] || begin[i] > bounds[1][i])
				return false;
		} else {								// near and far plane intersection points of this slab
			float dirInv = 1.0f / dir[i];
			float t1 = (bounds[0][i] - begin[i]) * dirInv;
			float t2 = (bounds[1][i] - begin[i]) * dirInv;
			if (t1 > t2)
				std::swap(t1, t2);

			tFirst = MAX(tFirst, t1);
			tLast = MIN(tFirst, t2);
			if (tFirst > tLast || tFirst > length)		// too disjoint, no intersection
				return false;
		}
	}

	result.fraction = tFirst;
	eVec2 hitPoint = begin + dir * tFirst;
	GetCollisionNormal(hitPoint, bounds, result);
	return true;
}

//***************
// eCollision::RayCast
// returns true and sets result to first collision along the ray (directed line segment)
// returns false for no collision and doesn't modify input result
// DEBUG: dir must be unit length
// TODO: add a collision mask to ignore certain colliders
//***************
bool eCollision::RayCast(Collision_t & result, const eVec2 & begin, const eVec2 & dir, const float length) {
	static std::unordered_map<const eCollisionModel *, const eCollisionModel *> alreadyTested;
	static std::vector<eGridCell *> broadAreaCells;					// DEBUG(performance): static to reduce dynamic allocations

	bool hit = false;
	GetAreaCells(begin, dir, length, broadAreaCells);
	for (auto & cell : broadAreaCells) {
//		if (!RayAABBTest(begin, dir, length, cell->AbsBounds(), result))		// will it collide with the cell
//			continue;															// DEBUG: GetAreaCells already means ray collides with these cells

		for (auto & kvPair : cell->Contents()) {
			auto & collider = kvPair.second;

			// don't test the same collider twice
			if (alreadyTested.find(collider) != alreadyTested.end())
				continue;

			alreadyTested[collider] = collider;
			if (RayAABBTest(begin, dir, length, collider->AbsBounds(), result)) {
				// TODO: track the collision with the lowest fraction,
				// figure out an early out (eg: cells sorted by distance from begin) (eg: if row has increased, then fraction will be larger)
				hit = true;
			}
		}
	}
	alreadyTested.clear();
	broadAreaCells.clear();
	return hit;					// return true if result was updated, false if not
}