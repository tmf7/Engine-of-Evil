#include "Game.h"

//*************
// eCollisionModel::~eCollisionModel
//*************
eCollisionModel::~eCollisionModel() {
	ClearAreas();
}

//*************
// eCollisionModel::Update
// TODO: move velocity to physics/rigidbody class (this will affect eMovementPlanner logic)
//*************
void eCollisionModel::Update() {
	if (active)
		AvoidCollisionSlide();		// TODO: alternatively push the collider away if it can be moved (non-static)

	oldOrigin = origin;
	origin = owner->orthoOrigin;
	origin += velocity; // * game.GetFixedTime();
	absBounds = localBounds + origin + orthoOriginOffset;
	center = absBounds.Center();

	// The engines don't move the ship at all. The ship stays where it is and the engines move the universe around it.
	owner->orthoOrigin = origin;

	if (active && origin != oldOrigin)
		UpdateAreas();
}

//*************
// eCollisionModel::SetOrigin
//*************
void eCollisionModel::SetOrigin(const eVec2 & newOrigin) {
	owner->SetOrigin(newOrigin);
}

//***************
// eCollisionModel::ClearAreas
// removes this from all eMap::tileMap gridcells with pointers to it
// and clear this->areas gridcell pointers
//***************
void eCollisionModel::ClearAreas() {
	for (auto && cell : areas) {
		auto & contents = cell->CollisionContents();
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
// adds this to the eMap::tileMap gridcells it overlaps
// and adds those same gridcell pointers to this->areas
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
	} else {							// BUGFIX: edge case where bounds matches its cell and winds up adding 4 areas instead of 1
		areas.emplace_back(&cell);
	}

	for (auto && cell : areas) {
		cell->CollisionContents()[this] = this;
	}
}

//***************
// eCollisionModel::FindApproachingCollision
// returns true and sets result to the nearest non-tangential collision along dir * length
// returns false and leaves result unmodified otherwise
// DEBUG: dir must be unit length
//***************
bool eCollisionModel::FindApproachingCollision(const eVec2 & dir, const float length, Collision_t & result) const {
	static std::vector<Collision_t> collisions;		// FIXME(~): make this a private data member instead of per-fn, if more than one fn uses it

	collisions.clear();	// DEBUG: lazy clearing
	if(eCollision::BoxCast(collisions, absBounds, dir, length)) {
		for (auto & collision : collisions) {
			float movingAway = collision.normal * dir;
			float movingAwayThreshold = ((abs(collision.normal.x) < 1.0f && abs(collision.normal.y) < 1.0f) ? -0.707f : 0.0f); // vertex : edge
			if (movingAway >= movingAwayThreshold) {
				continue;
			} else {
				result = collision;
				return true;
			}
		}
	}
	return false;
}

//***************
// eCollisionModel::AvoidCollisionCorrection
// truncates current velocity to prevent intersection with other colliders 
//***************
void eCollisionModel::AvoidCollisionCorrection() {
	if (velocity == vec2_zero)
		return;	

	Collision_t collision;
	float length = velocity.Length();
	if(FindApproachingCollision(velocity / length, length, collision))
		velocity *= collision.fraction;
}

//***************
// eCollisionModel::AvoidCollisionSlide
// truncates current velocity to prevent intersection with other colliders,
// or rotates velocity to slide along a touched collider
//***************
void eCollisionModel::AvoidCollisionSlide() {
	if (velocity == vec2_zero)
		return;

	Collision_t collision;
	eVec2 collisionTangent;
	float remainingFraction = 0.0f;
	float length = velocity.Length();

	if (FindApproachingCollision(velocity / length, length, collision)) {
		remainingFraction = 1.0f - collision.fraction;
		if (remainingFraction < 1.0f) {
			velocity *= collision.fraction;
		} else {
			// DEBUG: collisions sorted w/edges first for equal fractions
			collisionTangent = eVec2(-collision.normal.y, collision.normal.x);		// CCW 90 degrees
			float whichWay = collisionTangent * velocity;
			if (whichWay < 0)
				collisionTangent *= -1.0f;
		} 
	}

	if (remainingFraction == 1.0f) {
		float slide = velocity * collisionTangent;
		velocity = collisionTangent * slide;	
		if (FindApproachingCollision(collisionTangent, slide, collision))
			velocity *= collision.fraction;
	}
}