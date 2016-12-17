#ifndef ENTITY_H
#define ENTITY_H

#include "Definitions.h"
#include "Sprite.h"
#include "Bounds.h"
#include "Vector.h"

//*************************************************
//
//					eEntity
//
//*************************************************
class eEntity {
public:

						eEntity();

	virtual bool		Spawn();
	virtual void		Think() = 0;						// TODO: give this a definition, not pure virtual
	virtual void		Draw();								// FIXME: a eCamera is an eEntity, but it shouldn't Draw (have Think add things to a RenderQueue?)
	void				SetOrigin(const eVec2 & point);		// TODO: make this part of a physics class
	const eVec2 &		Origin() const;						// TODO: make this part of a physics class
	void				UpdateOrigin();						// TODO: make this part of a physics class

//	static eVec2 waypointPool[MAX_WAYPOINTS];		// TODO: use heap allocator class for new and delete to setup objects like this, 
													// instead of letting local variables go out of scope
//	static int activePoolIndex;

protected:

	eSprite				sprite;	
	eBounds				localBounds;			// using local coordinates
	eBounds				absBounds;				// using map coordinates
	eVec2				origin;
	eVec2				oldOrigin;				// for use with collision response
	eVec2				velocity;
	float				speed;
	float				maxSpeed;
};

//***************
// eEntity::eEntity
//***************
inline eEntity::eEntity() {
	maxSpeed	= 10.0f;
	speed		= maxSpeed;
}

//*************
// eEntity::UpdateOrigin
//*************
inline void eEntity::UpdateOrigin() {
	oldOrigin = origin;
	origin += velocity;
	absBounds = localBounds + origin;
}

//*************
// eEntity::SetOrigin
//*************
inline void eEntity::SetOrigin(const eVec2 & point) {
	oldOrigin = point;
	origin = point;
	absBounds = localBounds + origin;
}

//*************
// eEntity::Origin
//*************
inline const eVec2 & eEntity::Origin() const {
	return origin;
}

#endif /* ENTITY_H */