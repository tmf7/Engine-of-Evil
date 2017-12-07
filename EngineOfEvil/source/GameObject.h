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

	// FIXME: don't make component's access dependent on eGameObject definition
	// especially since derived classes don't get the same access
	friend class eCollisionModel;
	friend class eRenderImage;
	friend class eAnimationController;
	friend class eMovement;

public:
	
	virtual								   ~eGameObject() = default;
											eGameObject() = default;
											eGameObject(const eGameObject & other);
											eGameObject(eGameObject && other);
	eGameObject &							operator=(eGameObject other);


	virtual int								GetClassType() const override				{ return CLASS_GAMEOBJECT; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_GAMEOBJECT) 
													return true; 
												return eClass::IsClassType(classType); 
											}

	virtual void							Think()									{}
	virtual void							DebugDraw()								{}

	void									UpdateComponents();	
	const eVec2 &							GetOrigin()								{ return orthoOrigin; }
	void									SetOrigin(const eVec2 & newOrigin);
	Uint32									GetWorldLayer()							{ return worldLayer; }
	void									SetWorldLayer(Uint32 layer);
	void									SetWorldLayer(float zPosition);
	void									SetZPosition(float newZPosition);
	bool									IsStatic() const						{ return isStatic; }
	void									SetStatic(bool isStatic)				{ this->isStatic = isStatic; }

	bool									AddRenderImage(const std::string & spriteFilename, const eVec3 & renderBlockSize, int initialSpriteFrame = 0, const eVec2 & renderImageOffset = vec2_zero, bool isPlayerSelectable = false);
	bool									AddCollisionModel(const eBounds & localBounds, const eVec2 & colliderOffset = vec2_zero, bool collisionActive = false);
	bool									AddAnimationController(const std::string & animationControllerFilename);
	bool									AddMovementPlanner(float movementSpeed);

	// FIXME: make these proper const get, non-const set
	eRenderImage &							RenderImage()							{ return *renderImage; }
	eAnimationController &					AnimationController()					{ return *animationController; }
	eCollisionModel &						CollisionModel()						{ return *collisionModel; }
	eMovementPlanner &						MovementPlanner()						{ return *movementPlanner; }

private:

	void									UpdateComponentsOwner();

protected:

	std::unique_ptr<eRenderImage>			renderImage			= nullptr;		// data relevant to the renderer
	std::unique_ptr<eAnimationController>	animationController = nullptr;		// manipulates the renderImage
	std::unique_ptr<eCollisionModel>		collisionModel		= nullptr;		// handles collision between bounding volumes
	std::unique_ptr<eMovementPlanner>		movementPlanner		= nullptr;		// seeks goals by setting collisionModel::velocity
	
private:

	eVec2									orthoOrigin;						// orthographic 2D global transfrom coordinates
	float									zPosition			= 0.0f;			// 3D position used for fluid renderBlock positioning (TODO: and other 3D related tasks)
	Uint32									worldLayer			= MAX_LAYER;	// common layer on the eMap::tileMap (can position renderBlock and TODO: filters collision)
	Uint32									oldWorldLayer		= MAX_LAYER;	// helps track changes and minimize calculations
	bool									isStatic			= true;			// if orthoOrigin ever changes at runtime, speeds up draw-order sorting
};

#endif /* EVIL_GAMEOBJECT_H */

