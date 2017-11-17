#ifndef EVIL_GAMEOBJECT_H
#define EVIL_GAMEOBJECT_H

#include "RenderImage.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "AnimationController.h"

//*************************************************
//				eGameObject
// base composite-class for isolating eComponent groups
// and allowing meaningful intra- and inter-group communication
// handles the lifetime of all eComponent objects
// TODO: create AddComponent, RemoveComponent, and GetComponent templates
// to allow more flexible eGameObject extension
//*************************************************
class eGameObject : public eClass {
public:
									
											eGameObject() = default;
											eGameObject(const eGameObject & other);
											eGameObject(eGameObject && other);
	eGameObject &							operator=(eGameObject other);


	virtual int								GetClassType() const override	{ return CLASS_GAMEOBJECT; }
	virtual void							Think()							{}
	virtual void							DebugDraw()						{}

	eVec2 &									OrthoOrigin()					{ return orthoOrigin; }
	Uint32 &								WorldLayer()					{ return worldLayer; }
	eRenderImage &							RenderImage()					{ return *renderImage; }
	eAnimationController &					AnimationController()			{ return *animationController; }
	eCollisionModel &						CollisionModel()				{ return *collisionModel; }
	eMovementPlanner &						MovementPlanner()				{ return *movementPlanner; }
	bool &									IsStatic() 						{ return isStatic; }	

protected:

	void									UpdateComponentsOwner();

protected:

	eVec2									orthoOrigin;						// orthographic 2D global transfrom coordinates

	std::unique_ptr<eRenderImage>			renderImage			= nullptr;		// data relevant to the renderer
	std::unique_ptr<eAnimationController>	animationController = nullptr;		// manipulates the renderImage
	std::unique_ptr<eCollisionModel>		collisionModel		= nullptr;		// handles collision between bounding volumes
	std::unique_ptr<eMovementPlanner>		movementPlanner		= nullptr;		// seeks goals by setting collisionModel::velocity
	
	bool									isStatic			= true;			// if orthoOrigin ever changes at runtime, speeds up draw-order sorting
	Uint32									worldLayer			= MAX_LAYER;	// z-position in the game world (positions renderBlock and TODO: filters collision)

};

#endif /* EVIL_GAMEOBJECT_H */

