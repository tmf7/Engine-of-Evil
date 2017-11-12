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
// eCollision::SegmentAABBTest
// returns true if line segment overlaps bounds (includes touching)
// TODO: this same test can be used for a Segment vs OBB if the Segment is transformed into OOB-space
//***************
bool eCollision::SegmentAABBTest(const eVec2 & begin, const eVec2 & end, const eBounds & bounds) {
	// DEBUG: 0.5f factored out of setup variables 
	eVec2 extents = bounds[1] - bounds[0];
	eVec2 dir = end - begin;
	eVec2 midpoint = begin + end - bounds[0] - bounds[1];

	float adx = abs(dir.x);
	if (abs(midpoint.x) > extents.x + adx)
		return false;
	
	float ady = abs(dir.y);
	if (abs(midpoint.y) > extents.y + ady)
		return false;

	// counteract zero-value cross-product if segment is parallel to an axis
	adx += FLT_EPSILON;
	ady += FLT_EPSILON;

	// only one non-(0 > 0) separating axis test in 2D via cross-product of dir and coordinate axes
	if(abs(midpoint.x * dir.y - midpoint.y * dir.x) > extents.x * ady + extents.y * adx) 
		return false;
	return true;
}
		
//***************
// eCollision::BoxCast
// fills collisions according to AABBs intersected by bounds along (dir * length)
// sorted from nearest to farthest (collision.fractions in range [0.0f, 1.0f])
// returns true if any collision occurs
// DEBUG: dir must be unit length
// TODO: add a collision mask param to filter collisions
//***************
bool eCollision::BoxCast(std::vector<Collision_t> & collisions, const eBounds & bounds, const eVec2 & dir, const float length) {
	static std::unordered_map<const eBounds *, const eBounds *> alreadyTested;
	static std::vector<eGridCell *> broadAreaCells;					// DEBUG(performance): static to reduce dynamic allocations

	GetAreaCells(bounds, dir, length, broadAreaCells);
	alreadyTested[&bounds] = &bounds;								// ignore self collision
	for (auto & cell : broadAreaCells) {
		for (auto && kvPair : cell->CollisionContents()) {
			auto & collider = kvPair.second;
			const auto & otherBounds = &collider->AbsBounds();

			// don't test the same collider twice
			if (alreadyTested.find(otherBounds) != alreadyTested.end())
				continue;

			alreadyTested[otherBounds] = otherBounds;
			Collision_t collision;
			if (MovingAABBAABBTest(bounds, dir, length, *otherBounds, collision.fraction)) {
				collision.owner = collider;
				GetCollisionNormal(bounds, dir, length, *otherBounds, collision);
				collisions.push_back(collision);
			}
		}
	}
	alreadyTested.clear();
	broadAreaCells.clear();

	// DEBUG: prioritize edge collisions over vertex collisions with the same fraction
	QuickSort(	collisions.data(), 
				collisions.size(), 
				[](auto && a, auto && b) {
					if (a.fraction < b.fraction) return -1;
					else if (a.fraction > b.fraction) return 1;
					else if (!(abs(a.normal.x) < 1.0f && abs(a.normal.y) < 1.0f) && 
							  (abs(b.normal.x) < 1.0f && abs(b.normal.y) < 1.0f)) return -1;
					else if ((abs(a.normal.x) < 1.0f && abs(a.normal.y) < 1.0f) && 
							 !(abs(b.normal.x) < 1.0f && abs(b.normal.y) < 1.0f)) return 1;
					return 0;
	});
	return !collisions.empty();
}

//***************
// eCollision::GetAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// within the given area box (includes touching)
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using a reserved/managed vector, or static memory
//***************
void eCollision::GetAreaCells(const eBox & area, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	static std::deque<eGridCell *> openSet;		// first-come-first-served testing
	static std::vector<eGridCell *> closedSet;
	static std::vector<eGridCell *> neighbors;
	
	auto & initialCell = tileMap.IndexValidated(area.Center());
	openSet.push_back(&initialCell);
	initialCell.inOpenSet = true;

	while(!openSet.empty()) {
		auto cell = openSet.front();
		openSet.pop_front();		
		closedSet.push_back(cell);
		cell->inOpenSet = false;
		cell->inClosedSet = true;

		if (OBBOBBTest(area, eBox(cell->AbsBounds()))) {
			areaCells.push_back(cell);
			tileMap.GetNeighbors(cell->GridRow(), cell->GridColumn(), neighbors);

			for(auto & neighborCell : neighbors) {
				if (!neighborCell->inClosedSet && !neighborCell->inOpenSet) {
					openSet.push_back(neighborCell);
					neighborCell->inOpenSet = true;
				}
			}
			neighbors.clear();
		}
	}

	for (auto & cell : closedSet)
		cell->inClosedSet = false;
	closedSet.clear();
}

//***************
// eCollision::GetAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// within the given area bounds (includes touching)
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using a reserved/managed vector, or static memory
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
// along the given area swept by the bounds along (dir * length) (includes touching)
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using a reserved/managed vector, or static memory
//***************
void eCollision::GetAreaCells(const eBounds & bounds, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	static std::deque<eGridCell *> openSet;		// first-come-first-served testing
	static std::vector<eGridCell *> closedSet;
	static std::vector<eGridCell *> neighbors;
	
	auto & initialCell = tileMap.IndexValidated(bounds.Center());
	openSet.push_back(&initialCell);
	initialCell.inOpenSet = true;

	float placeholderFraction;					// DEBUG: not used
	while(!openSet.empty()) {
		auto cell = openSet.front();
		openSet.pop_front();		
		closedSet.push_back(cell);
		cell->inOpenSet = false;
		cell->inClosedSet = true;

		if (MovingAABBAABBTest(bounds, dir, length, cell->AbsBounds(), placeholderFraction)) {
			areaCells.push_back(cell);
			tileMap.GetNeighbors(cell->GridRow(), cell->GridColumn(), neighbors);

			for(auto & neighborCell : neighbors) {
				if (!neighborCell->inClosedSet && !neighborCell->inOpenSet) {
					openSet.push_back(neighborCell);
					neighborCell->inOpenSet = true;
				}
			}
			neighbors.clear();
		}
	}

	for (auto & cell : closedSet)
		cell->inClosedSet = false;
	closedSet.clear();
}

//***************
// eCollision::GetAreaCells
// fills the areaCells vector with pointers to the eGridCells 
// along the given ray (directed line segment) (includes touching)
// DEBUG(performance): make sure areaCells passed in avoids excessive dynamic allocation by using a reserved/managed vector, or static memory
//***************
void eCollision::GetAreaCells(const eVec2 & begin, const eVec2 & dir, const float length, std::vector<eGridCell *> & areaCells) {
	auto & tileMap = game.GetMap().TileMap();
	static std::deque<eGridCell *> openSet;		// first-come-first-served testing
	static std::vector<eGridCell *> closedSet;
	static std::vector<eGridCell *> neighbors;
	
	auto & initialCell = tileMap.IndexValidated(begin);
	openSet.push_back(&initialCell);
	initialCell.inOpenSet = true;

	float placeholderFraction;					// DEBUG: not used
	while(!openSet.empty()) {
		auto cell = openSet.front();
		openSet.pop_front();		
		closedSet.push_back(cell);
		cell->inOpenSet = false;
		cell->inClosedSet = true;

		if (RayAABBTest(begin, dir, length, cell->AbsBounds(), placeholderFraction)) {
			areaCells.push_back(cell);
			tileMap.GetNeighbors(cell->GridRow(), cell->GridColumn(), neighbors);

			for(auto & neighborCell : neighbors) {
				if (!neighborCell->inClosedSet && !neighborCell->inOpenSet) {
					openSet.push_back(neighborCell);
					neighborCell->inOpenSet = true;
				}
			}
			neighbors.clear();
		}
	}

	for (auto & cell : closedSet)
		cell->inClosedSet = false;
	closedSet.clear();
}

//***************
// eCollision::BroadPhaseBounds
// generates an AABB that encompasses
// the swept area of a moving eBounds along (dir * length)
//***************
eBounds eCollision::GetBroadPhaseBounds(const eBounds & bounds, const eVec2 & dir, const float length) {
	eVec2 velocity = dir * length;
	eBounds bpBounds;
	if (velocity.x > 0.0f) {
		bpBounds[0][0] = bounds[0][0];
		bpBounds[1][0] = bounds[1][0] + velocity.x;
	} else {
		bpBounds[0][0] = bounds[0][0] + velocity.x;
		bpBounds[1][0] = bounds[1][0] - velocity.x;
	}
		
	if (velocity.y > 0.0f) {
		bpBounds[0][1] = bounds[0][1];
		bpBounds[1][1] = bounds[1][1] + velocity.y;
	} else {
		bpBounds[0][1] = bounds[0][1] + velocity.y;
		bpBounds[1][1] = bounds[1][1] - velocity.y;
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
// returns true and sets resultFraction along self.velocity where touching first occurs with other
// returns false for no collision and doesn't modify resultFraction
// DEBUG: resultFraction will be [0.0f, 1.0f] of length after confirmed collision
// DEBUG: includes touching at the very end of dir * length (ie fraction == 1.0f)
//***************
bool eCollision::MovingAABBAABBTest(const eBounds & self, const eVec2 & dir, const float length, const eBounds & other, float & resultFraction) {
	if (AABBAABBTest(self, other)) {	// started in collision
		resultFraction = 0.0f;
		return true;
	}

	eVec2 selfMin = self[0];
	eVec2 selfMax = self[1];
	eVec2 otherMin = other[0];
	eVec2 otherMax = other[1];
	eVec2 velocity = dir * length + vec2_epsilon;	// DEBUG: account for axis-parallel travel, and divide-by-zero
	float tFirst = 0.0f;
	float tLast = 1.0f;

	// determine times of first and last contact, if any
	for (int i = 0; i < 2; i++) {
		if (velocity[i] < 0.0f) {
			if (selfMax[i] < otherMin[i]) return false; // non-intersecting and moving apart
			if (otherMax[i] < selfMin[i]) tFirst = MAX((otherMax[i] - selfMin[i]) / velocity[i], tFirst);
			if (selfMax[i] > otherMin[i]) tLast = MIN((otherMin[i] - selfMax[i]) / velocity[i], tLast);
		}
		if (velocity[i] > 0.0f) {
			if (selfMin[i] > otherMax[i]) return false; // non-intersecting and moving apart
			if (selfMax[i] < otherMin[i]) tFirst = MAX((otherMin[i] - selfMax[i]) / velocity[i], tFirst);
			if (otherMax[i] > selfMin[i]) tLast = MIN((otherMax[i] - selfMin[i]) / velocity[i], tLast);
		}
		
		// too disjoint to make contact
		if (tFirst > tLast)
			return false;
	}
	resultFraction = tFirst;
	return true;		// intersecting
}

//***************
// eCollision::GetCollisionNormal
// sets the collision's surface-normal based on point's position on bounds
// DEBUG: does not predict collision normal if not yet in contact
// DEBUG: this only works for AABB, not OBB or general convex polygons
// DEBUG: not touching, or overlapping beyond touching is zero normal vector
//***************
void eCollision::GetCollisionNormal(const eVec2 & point, const eBounds & bounds, eVec2 & resultNormal) {
	Uint8 entryDir = (RIGHT * (abs(point.x - bounds[0].x) == 0.0f)) 
					| (LEFT * (abs(point.x - bounds[1].x) == 0.0f)) 
					| (TOP * (abs(point.y - bounds[0].y) == 0.0f)) 
					| (BOTTOM * (abs(point.y - bounds[1].y) == 0.0f));
	SetAABBNormal(entryDir, resultNormal);
}

//***************
// eCollision::GetCollisionNormal
// sets the collision's surface-normal based on self's relative position to other
// predicts collision normal if not yet in contact
// TODO: trigger colliders (ie: allowing traversal) may need exit fraction and normal information
// DEBUG: this only works for AABB, not OBB or general convex polygons
// DEBUG: overlapping beyond touching is zero normal vector
//***************
void eCollision::GetCollisionNormal(eBounds self, const eVec2 & dir, const float length, const eBounds & other, Collision_t & collision) {
	if (collision.fraction > 0.0f)
		self += dir * (length * collision.fraction);

	// FIXME: there are 4 (or 2) setups where dir must be considered to decide
	// if both an x and y dist are 0.0f

	eVec2 selfMin = self[0];
	eVec2 selfMax = self[1];
	eVec2 otherMin = other[0];
	eVec2 otherMax = other[1];
//	eVec2 velocity = dir * length + vec2_epsilon;	// DEBUG: account for axis-parallel travel, and divide-by-zero 

	// distance from self to other for normal on other's surface
	float xRightEntryDist = abs(selfMin.x - otherMax.x);
	float xLeftEntryDist = abs(otherMin.x - selfMax.x);
	float yTopEntryDist = abs(otherMin.y - selfMax.y);
	float yBottomEntryDist = abs(selfMin.y - otherMax.y);

	Uint8 entryDir = 0;
/*
	if (xRightEntryDist == 0.0f) {
		entryDir = RIGHT;
	} else if (xLeftEntryDist == 0.0f) {
		entryDir = LEFT;
	} else if (yTopEntryDist == 0.0f) {
		entryDir = TOP;
	} else if (yBottomEntryDist == 0.0f) {
		entryDir = BOTTOM;
	}
*/
//	if (collision.fraction == 0.0f) {
		entryDir |= (RIGHT * (xRightEntryDist == 0.0f)) 
					| (LEFT * (xLeftEntryDist == 0.0f)) 
					| (TOP * (yTopEntryDist == 0.0f)) 
					| (BOTTOM * (yBottomEntryDist == 0.0f));
/*
	} else {
		float xFraction = abs(MIN(xRightEntryDist, xLeftEntryDist) / velocity.x);
		float yFraction = abs(MIN(yTopEntryDist, yBottomEntryDist) / velocity.y);
		
		if (xFraction == yFraction) {
			entryDir |=	(xRightEntryDist < xLeftEntryDist ? RIGHT : LEFT);
			entryDir |= (yTopEntryDist < yBottomEntryDist ? TOP : BOTTOM);
		} else if (xFraction < yFraction) {
			entryDir |=	(xRightEntryDist < xLeftEntryDist ? RIGHT : LEFT);
		} else {
			entryDir |= (yTopEntryDist < yBottomEntryDist ? TOP : BOTTOM);
		}
	}
*/
	SetAABBNormal(entryDir, collision.normal);
}

//***************
// eCollision::RayAABBTest
// ray: Point = begin + t * dir
// plane: Point * normal = distFromOrigin
// returns true and sets resultFraction along dir where touching first occurs with bounds (includes touching)
// returns false for no collision and doesn't modify resultFraction
// DEBUG: resultFraction will be [0, length] after confirmed collision
// DEBUG: dir must be unit length
//***************
bool eCollision::RayAABBTest(const eVec2 & begin, const eVec2 & dir, const float length, const eBounds & bounds, float & resultFraction) {
	float tFirst = 0.0f;
	float tLast = FLT_MAX;

	for (int i = 0; i < 2; ++i) {
		if (abs(dir[i]) < FLT_EPSILON) {			// parallel to both planes of this slab
			if (begin[i] < bounds[0][i] || begin[i] > bounds[1][i])
				return false;
		} else {									// near and far plane intersection points of this slab
			float dirInv = 1.0f / dir[i];
			float t1 = (bounds[0][i] - begin[i]) * dirInv;
			float t2 = (bounds[1][i] - begin[i]) * dirInv;
			if (t1 > t2)
				std::swap(t1, t2);

			tFirst = MAX(tFirst, t1);
			tLast = MIN(tFirst, t2);
			if (tFirst > tLast || tFirst > length)	// non-intersecting, too disjoint
				return false;
		}
	}

	resultFraction = tFirst;
	return true;
}

//***************
// eCollision::RayCast
// fills collisions according to AABBs intersected by the ray (directed line segment)
// sorted from nearest to farthest (collision.fraction in range [0.0f, length])
// returns true if any collision occurs
// DEBUG: dir must be unit length
// TODO: add a collision mask param to filter collisions
//***************
bool eCollision::RayCast(std::vector<Collision_t> & collisions, const eVec2 & begin, const eVec2 & dir, const float length, bool ignoreStartInCollision) {
	static std::unordered_map<const eCollisionModel *, const eCollisionModel *> alreadyTested;
	static std::vector<eGridCell *> broadAreaCells;							// DEBUG(performance): static to reduce dynamic allocations

	GetAreaCells(begin, dir, length, broadAreaCells);
	for (auto & cell : broadAreaCells) {
		for (auto & kvPair : cell->CollisionContents()) {
			auto & collider = kvPair.second;

			// don't test the same collider twice
			if (alreadyTested.find(collider) != alreadyTested.end())
				continue;

			alreadyTested[collider] = collider;
			Collision_t collision;
			if (RayAABBTest(begin, dir, length, collider->AbsBounds(), collision.fraction) && !(collision.fraction == 0.0f && ignoreStartInCollision)) {
				collision.owner = collider;
				eVec2 touchPoint = begin + dir * collision.fraction;
				GetCollisionNormal(touchPoint, collider->AbsBounds(), collision.normal);
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