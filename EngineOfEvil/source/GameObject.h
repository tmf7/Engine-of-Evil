/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_GAMEOBJECT_H
#define EVIL_GAMEOBJECT_H

#include "RenderImage.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "AnimationController.h"

class eMap;

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
	friend class eMovementPlanner;

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

	virtual void							Init()									{}
	virtual void							Think()									{}
	virtual void							DebugDraw(eRenderTarget * renderTarget)	{}

	void									UpdateComponents();	
	eMap * const							GetMap()								{ return map; }
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

	eCollisionModel &						CollisionModel()						{ return *collisionModel; }
	const eCollisionModel &					CollisionModel() const					{ return *collisionModel; }

	// FIXME: make these proper const get, non-const set
	eRenderImage &							RenderImage()							{ return *renderImage; }
	eAnimationController &					AnimationController()					{ return *animationController; }
	eMovementPlanner &						MovementPlanner()						{ return *movementPlanner; }


private:

	void									UpdateComponentsOwner();

protected:

	eMap *									map;								// back-pointer to the eMap object owns *this
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

