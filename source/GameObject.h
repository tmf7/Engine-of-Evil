#ifndef EVIL_GAMEOBJECT_H
#define EVIL_GAMEOBJECT_H

#include "RenderImage.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "Sprite.h"

//*************************************************
//				eGameObject
// base class for all in-game interactive objects
// TODO: handles the lifetime of all eComponent objects (eRenderImage, eCollisionModel, eMovementPlanner, eSprite)
//*************************************************
class eGameObject : public eClass {
private:

//	friend class eComponent;			// TODO: implement this class and give *this a std::vector<eComponent>

public:

	virtual void							Think()							{};
	virtual void							DebugDraw()						{};
	virtual int								GetClassType() const override	{ return CLASS_GAMEOBJECT; }

	eRenderImage &							RenderImage()					{ return *renderImage; }
	eSprite &								AnimationController()			{ return *animationController; }
	eCollisionModel &						CollisionModel()				{ return *collisionModel; }
	eMovementPlanner &						MovementPlanner()				{ return *movementPlanner; }
	bool									IsStatic() const				{ return isStatic; }
	void									SetIsStatic(bool doesntMove)	{ isStatic = doesntMove; }			

private:

	eVec2									origin;					// orthographic 2D global transfrom coordinates

	std::unique_ptr<eRenderImage>			renderImage;			// data relevant to the renderer
	std::unique_ptr<eSprite>				animationController;	// TODO: rename to eAnimationController, and use it to manipulate the eRenderImage
	std::unique_ptr<eCollisionModel>		collisionModel;	
	std::unique_ptr<eMovementPlanner>		movementPlanner;
	
	bool									isStatic;				// whether the origin ever changes at runtime, speeds up draw-order sorting

};

#endif /* EVIL_GAMEOBJECT_H */

