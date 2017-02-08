#include "Collision.h"
#include "CollisionModel.h"
#include "Game.h"

//***************
// eCollision::ForwardCollisionTest
// returns the fraction along the current velocity (0.0f, 1.0f] 
// where touching first occurs between self and 
// one of areaContents (ie hitCollider), if any
// DEBUG: never returns 0.0f which implies two things started in overlap/touching
//***************
bool eCollision::ForwardCollisionTest(const eCollisionModel & self, const std::vector<eCollisionModel *> & areaContents, std::vector<Collision_t> & collisions) {
	eBounds broadPhaseBounds = GetBroadPhaseBounds(self);
	for (auto && collider : areaContents) {

		// broad-phase test first
		// TODO: check if the broadphasebounds collides with the CELL before testing collision with its contents
		// THEN ALSO DO broadPhase against each of the contents
		if (AABBAABBTest(broadPhaseBounds, collider->Bounds())) {
			eVec2 normal;
			float fraction = MovingAABBAABBTest(self, *collider, normal);
			collisions.push_back(Collision_t{normal, fraction, collider});
		}
	}
	return collisions.size() > 0;
}

//***************
// eCollision::GetAreaContents
// fills the areaContents vector with 
// pointers to the eCollisionModels 
// in and of the 9 cells centered on centerPoint
// DEBUG: make the passed areaContents function-static
// to avoid excessive dynamic allocation
//***************
void eCollision::GetAreaContents(const eVec2 & centerPoint, std::vector<eCollisionModel *> & areaContents) {
	auto & tileMap = game.GetMap().TileMap();
	int centerRow;
	int centerCol;
	tileMap.Index(centerPoint, centerRow, centerCol);

	for (int row = -1; row <= 1; row++) {
		for (int col = -1; col <= 1; col++) {
			int checkRow = centerRow + row;
			int checkCol = centerCol + col;
			if (!tileMap.IsValid(checkRow, checkCol))
				continue;

			auto & cell = tileMap.Index(checkRow, checkCol);

			// add the cell broadPhaseCollider that's somehow just a bounds
			// AS WELL AS a collider (or something) for broad phase cell tests
			// FIXME/BUG: prune duplicates AFTER all have been added?
			// FIXME/BUG: a cells contents includes its tiles' colliders already (as well as potentially some other cells' tiles' colliders)
			for (auto && tile : cell.Tiles())
				areaContents.push_back(&tile.Collider());

			// add the colliders of the tile's contents
			for (auto && collider : cell.Contents()) {

				// don't add the same collider twice for those overlapping multiple tiles
				auto & exists = std::find(areaContents.begin(), areaContents.end(), collider);
				if (exists == areaContents.end()) {
					areaContents.push_back(collider);
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
		bpBounds[0][0] = self.Bounds()[0][0];
		bpBounds[1][0] = self.Bounds()[1][0] + self.Velocity().x;
	} else {
		bpBounds[0][0] = self.Bounds()[0][0] + self.Velocity().x;
		bpBounds[1][0] = self.Bounds()[1][0] - self.Velocity().x;
	}
		
	if (self.Velocity().y > 0.0f) {
		bpBounds[0][1] = self.Bounds()[0][1];
		bpBounds[1][1] = self.Bounds()[1][1] + self.Velocity().y;	
	} else {
		bpBounds[0][1] = self.Bounds()[0][1] + self.Velocity().y;
		bpBounds[1][1] = self.Bounds()[1][1] - self.Velocity().y;
	}
	return bpBounds;
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
// returns 0.0f if already in collision
// returns 1.0f if no collision will occur
// otherwise returns the fraction along the movement
// where collision first occurs
// DEBUG: includes touching and starting in collision
// FIXME/BUG: THIS FUNCTION AND THE UTILITIES THAT CALL IT STILL NEED WORK (based on "Dont Be A Hero" project)
//***************
float eCollision::MovingAABBAABBTest(const eCollisionModel & self, const eCollisionModel & other, eVec2 & collisionNormal) {
	const eVec2 & selfMin = self.Bounds()[0];
	const eVec2 & selfMax = self.Bounds()[1];
	const eVec2 & otherMin = other.Bounds()[0];
	const eVec2 & otherMax = other.Bounds()[1];
	eVec2 relativeVel = self.Velocity() - other.Velocity();
	float tFirst = 0.0f;
	float tLast = 1.0f;
	collisionNormal = vec2_zero;

	// started in collision (including touching)
	if (AABBAABBTest(self.Bounds(), other.Bounds())) {
		// TODO: set the normal based on relative velocity and positions...just like the main for loop does
	
	}


	// determine times of first and last contact, if any
	for (int i = 0; i < 2; i++) {
		if (relativeVel[i] < 0.0f) {
			if (selfMax[i] < otherMin[i]) {	// non-intersecting and moving apart
				collisionNormal = vec2_zero;
				return 1.0f;
			}
			if (otherMax[i] < selfMin[i]) { 
				tFirst = eMath::Maximize((otherMax[i] - selfMin[i]) / relativeVel[i], tFirst);
				collisionNormal[i] = 1.0f;
			}
			if (selfMax[i] > otherMin[i]) {
				tLast = eMath::Minimize((otherMin[i] - selfMax[i]) / relativeVel[i], tLast);
			}
		}
		if (relativeVel[i] > 0.0f) {
			if (selfMin[i] > otherMax[i]) {	// non-intersecting and moving apart
				collisionNormal = vec2_zero;
				return 1.0f;
			}
			if (selfMax[i] < otherMin[i]) {
				tFirst = eMath::Maximize((otherMin[i] - selfMax[i]) / relativeVel[i], tFirst);
				collisionNormal[i] = -1.0f;
			}
			if (otherMax[i] > selfMin[i]) {
				tLast = eMath::Minimize((otherMax[i] - selfMin[i]) / relativeVel[i], tLast);
			}
		}

		// generally, too far away to make contact
		if (tFirst > tLast) {
			collisionNormal = vec2_zero;
			return 1.0f;
		}
	}
	return tFirst;
}