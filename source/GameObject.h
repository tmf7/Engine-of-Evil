#ifndef EVIL_GAMEOBJECT_H
#define EVIL_GAMEOBJECT_H

#include "RenderImage.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "AnimationController.h"

//*************************************************
//				eGameObject
// base class for all in-game interactive objects
// handles the lifetime of all eComponent objects
//*************************************************
class eGameObject : public eClass {
public:
									
											eGameObject() = default;
											eGameObject(const eGameObject & other);
											eGameObject(eGameObject && other);
	eGameObject &							operator=(eGameObject other);


	virtual void							Think()							{};
	virtual void							DebugDraw()						{};
	virtual int								GetClassType() const override	{ return CLASS_GAMEOBJECT; }

	eVec2 &									TransformPosition()				{ return transformPosition; }
	eRenderImage &							RenderImage()					{ return *renderImage; }
	eAnimationController &					AnimationController()			{ return *animationController; }
	eCollisionModel &						CollisionModel()				{ return *collisionModel; }
	eMovementPlanner &						MovementPlanner()				{ return *movementPlanner; }
	bool &									IsStatic() 						{ return isStatic; }		
	int	&									WorldLayer()					{ return worldLayer; }

protected:

	eVec2									transformPosition;					// orthographic 2D global transfrom coordinates

	std::unique_ptr<eRenderImage>			renderImage			= nullptr;		// data relevant to the renderer
	std::unique_ptr<eAnimationController>	animationController = nullptr;		// manipulates the renderImage
	std::unique_ptr<eCollisionModel>		collisionModel		= nullptr;		// handles collision between bounding volumes
	std::unique_ptr<eMovementPlanner>		movementPlanner		= nullptr;		// seeks goals by setting collisionModel::velocity
	
	bool									isStatic			= true;			// if transformPosition ever changes at runtime, speeds up draw-order sorting
	int										worldLayer			= MAX_LAYER;	// z-position in the game world (positions renderImage and TODO: filters collision)

};

#endif /* EVIL_GAMEOBJECT_H */

