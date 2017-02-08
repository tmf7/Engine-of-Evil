#ifndef ENTITY_H
#define ENTITY_H

#include "Definitions.h"
#include "Sprite.h"
#include "CollisionModel.h"

//*************************************************
//
//					eEntity
// TODO: pass in spawnArgs: position, facing angle/unit-velocity, speed
//*************************************************
class eEntity {
public:

	virtual bool		Spawn();
	virtual void		Think() = 0;
	virtual void		Draw();		

protected:

	eSprite				sprite;	
	eCollisionModel		collider;
};

#endif /* ENTITY_H */