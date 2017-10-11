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

	renderImage_t				renderImage;			// data relevant to the renderer

	// DEBUG: collisionModel.Origin(), collisionModel.AbsBounds().Center and renderImage.origin 
	// are treated different in eEntity compared to eTile. 
	// --> eEntity moves via its collisionModel::origin so its localBounds is centered on (0,0)
	// and its renderImage.origin is positioned and offset (post-isometric-transformation) from the collisionModel.AbsBounds()[0] (minimum/top-left corner)
	// --> eTile does not move, and are positioned once according to its owner eGridCell position, then isometrically transformed
	// and its collisionModel.LocalBounds()[0] == renderImage.origin (pre-isometric-transformation) + eVec2(arbitraryOffset)
	// --> the reason this discrepancy currently remains is that eAI logic depends on a centered absBounds origin
	// TODO: update eAI logic to work from a universal eTransform::origin, and make
	// renderImage::origin, and collisionModel::origin positioned via offsets from that shared eTransform::origin
	// for both eTile and eEntity (static and dynamic objects)
	eVec2						imageColliderOffset;	

	eSprite						sprite;					// TODO: use this to manipulate the renderImage_t
	eCollisionModel				collisionModel;		

	// TODO: not all eEntity require visible sprites or collision models (mix and match composite instead of inheritence)
//	std::shared_ptr<eSprite>			optionalSprite;
//	std::shared_ptr<eCollisionModel>	optionalCollisionModel;
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