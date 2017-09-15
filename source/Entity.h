#ifndef ENTITY_H
#define ENTITY_H

#include "Definitions.h"
#include "Sprite.h"
#include "CollisionModel.h"
#include "Collision.h"
#include "Renderer.h"

//*************************************************
//					eEntity
//
//*************************************************
class eEntity {
public:
								eEntity();

	virtual bool				Spawn(/*const char * entityFilename, eVec2 & worldPosition*/);
	virtual void				Think() = 0;
	virtual void				Draw();
	virtual void				DebugDraw() = 0;
	virtual renderImage_t *		GetRenderImage();
	void						UpdateRenderImageOrigin();
	void						UpdateRenderImageDisplay();
	eCollisionModel &			CollisionModel();
	eSprite &					Sprite();

protected:

	renderImage_t				renderImage;		// data relevant to the renderer


	eSprite						sprite;				// TODO: soon to replace renderImage_t
	eCollisionModel				collisionModel;		

	// TODO: not all eEntity require visible sprites or collision models (mix and match composite instead of inheritence)
	std::shared_ptr<eSprite>			optionalSprite;
	std::shared_ptr<eCollisionModel>	optionalCollisionModel;

//	eVec2						imageOffset;		// force renderImage and collisionModel to misalign (TODO: put this in eCollisionModel and eSprite each, give origin to eEntity)
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