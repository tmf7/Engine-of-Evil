#ifndef ENTITY_H
#define ENTITY_H

#include "Definitions.h"
#include "Sprite.h"
#include "CollisionModel.h"
#include "Collision.h"
#include "Renderer.h"

//*************************************************
//					eEntity
// TODO: pass in spawnArgs: position, facing angle/unit-velocity, speed
// TODO: pass in spawnArgs: layer, initial animation (etc) for renderImage_t/eSprite
//*************************************************
class eEntity {
public:
								eEntity();

	virtual bool				Spawn();
	virtual void				Think() = 0;
	virtual void				Draw();
	virtual renderImage_t *		GetRenderImage();
	void						UpdateRenderImageOrigin();
	void						UpdateRenderImageDisplay();
	eCollisionModel &			CollisionModel();
	eSprite &					Sprite();

protected:

	renderImage_t				renderImage;		// data relevant to the renderer
	eSprite						sprite;	
	eCollisionModel				collisionModel;
//	eVec2						imageOffset;		// force renderImage and collisionModel to misalign
};

//**************
// eEntity::eEntity
//**************
inline eEntity::eEntity() {
}

//**************
// eEntity::CollisionModel
//**************
inline eCollisionModel & eEntity::CollisionModel() {
	return collisionModel;
}

//**************
// eEntity::Sprite
//**************
inline eSprite & eEntity::Sprite() {
	return sprite;
}

#endif /* ENTITY_H */