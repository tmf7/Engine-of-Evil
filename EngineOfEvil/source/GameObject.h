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

// TODO: replace these eComponent-derived header includes with just #include "eComponent.h" 
// once the generalized Add/Get/RemoveComponent methods are finalized
// to allow users to attach ANY new eComponent-derived object to *this
#include "RenderTarget.h"
#include "RenderImageBase.h"
#include "CollisionModel.h"
#include "Movement.h"
#include "AnimationController.h"
#include <typeindex>					// FIXME: no longer needed?

namespace evil { 

class eMap;

//*************************************************
//				eGameObject
// base composite-class for isolating eComponent groups
// and allowing meaningful intra- and inter-group communication
// handles the lifetime of all eComponent objects
// TODO: create a forwarding AddComponent, RemoveComponent, and GetComponent templates
// to allow more flexible eGameObject extension
// SOLUTION: GetComponent can either be an hashmap or just an array linear searched by typeid (or CLASS_classname)
// that returns the first element of the type (but can also return ALL elements of a type in the event of multiple)
// OR a eHashIndex to more quickly search (though components may arguably be a small vector anyway)
// SOLUTION: RemoveComponent would have to take a templated type...and or an index value w/in eGameObject::components
//*************************************************
class eGameObject : public eClass {

	ECLASS_DECLARATION(eGameObject)

public:
	
	virtual								   ~eGameObject() = default;
											eGameObject() = default;
											eGameObject(const eGameObject & other);
											eGameObject(eGameObject && other);
	eGameObject &							operator=(eGameObject other);


	virtual void							Think()									{}
	virtual void							DebugDraw(eRenderTarget * renderTarget)	{}

	void									UpdateComponents();	
	eMap * const							GetMap()								{ return map; }
	const eVec2 &							GetOrigin()								{ return orthoOrigin; }
	void									SetOrigin(const eVec2 & newOrigin)		{ orthoOrigin = newOrigin; }
	Uint32									GetWorldLayer()							{ return worldLayer; }
	void									SetWorldLayer(Uint32 layer);
	void									SetWorldLayer(float zPosition);
	void									SetZPosition(float newZPosition);
	float									GetZPosition() const					{ return zPosition; }
	bool									IsStatic() const						{ return isStatic; }
	void									SetStatic(bool isStatic)				{ this->isStatic = isStatic; }

/*

	std::vector<std::unique_ptr<eComponent>> components;

	// perfect-forwards all params to the ComponentType constructor with the matching parameter list
	// DEBUG: be sure to compare the arguments of this fn to the desired constructor to avoid perfect-forwarding failure cases
	// EG: initializer lists, decl-only static const int members, 0|NULL instead of nullptr, overloaded fn names, and bitfields
	template<class ComponentType, typename... Args>
	ComponentType *							AddComponent(Args&&... params) {

		// FIXME: potentially also add to a eHashIndex and allow collisions for base class types
		// such that a Get(base) could check the hashIndex and find derived and base a the same index...should work with many collisions (on one gameobject)
		// BUT only ONE emplace_back on the components vector
		auto & result = components.emplace_back( std::make_unique<ComponentType>( std::forward<Args>( params )... ) );
		return &result;
	}
	// FIXME: if this is a template fn then it will be more complex to traverse the class hierarchy 
	// (ie multiple adds to a hashIndex during AddComponent...using typeid(ComponentType) as the hash...or adding an <...int...> template param that takes the CLASS_XYZ int to hash it)
	// OR a funky call syntax would be necessary: const auto & ri = gameObject.GetComponent<eRenderImageBase>(CLASS_RENDERIMAGE_BASE);

	// UNLESS the ComponentType could have a static ClassType method 
	// (which is possible, but coordination of the return values may be complex given the new independence of each class [const hash of the classname?])
	// and just leave the IsClassType virtual???
	eComponent *							GetComponent(Uint32 classType) {
		eCollisionModel::Type;
		for ( auto && component : components ) {
			if (component->IsClassType(classType))
				return component.get();												// FIXME: the user will have to perform a static cast to the return value to use its derived fns
		}
		return nullptr;
	}

	void									RemoveComponent(Uint32 classType) {
		// FIXME: ComponentType may not be captured in the lambda?
		auto & index = std::find_if(components.begin(), components.end(), [classType](auto & component) { return component->IsClassType(classType); } );
		if (index != components.end())
			components.erase(index);
	}

	// GetComponent (const)
	// GetComponents (const & non-const)
	// RemoveComponents (all of a specific type (not base type))
*/

	bool									AddRenderImageBase(const std::string & spriteFilename, int initialSpriteFrame = 0, const eVec2 & renderImageOffset = vec2_zero, bool isPlayerSelectable = false);
	bool									AddRenderImageIsometric(const std::string & spriteFilename, const eVec3 & renderBlockSize, int initialSpriteFrame = 0, const eVec2 & renderImageOffset = vec2_zero, bool isPlayerSelectable = false);
	bool									AddCollisionModel(const eBounds & localBounds, const eVec2 & colliderOffset = vec2_zero, bool collisionActive = false);
	bool									AddAnimationController(const std::string & animationControllerFilename);
	bool									AddMovementPlanner(float movementSpeed);

	eCollisionModel &						CollisionModel()						{ return *collisionModel; }
	const eCollisionModel &					CollisionModel() const					{ return *collisionModel; }

	// FIXME: make these proper const get, non-const set
	eRenderImageBase &						RenderImage()							{ return *renderImage; }
	eAnimationController &					AnimationController()					{ return *animationController; }
	eMovementPlanner &						MovementPlanner()						{ return *movementPlanner; }


private:

	void									UpdateComponentsOwner();

protected:

	eMap *									map;								// back-pointer to the eMap object owns *this
	std::unique_ptr<eRenderImageBase>		renderImage			= nullptr;		// data relevant to the renderer
	std::unique_ptr<eAnimationController>	animationController = nullptr;		// manipulates the renderImage
	std::unique_ptr<eCollisionModel>		collisionModel		= nullptr;		// handles collision between bounding volumes
	std::unique_ptr<eMovementPlanner>		movementPlanner		= nullptr;		// seeks goals by setting collisionModel::velocity
	
private:

	eVec2									orthoOrigin;						// orthographic 2D global transfrom coordinates
	float									zPosition			= 0.0f;			// 3D position used for fluid renderBlock positioning (TODO: and other 3D related tasks)
	Uint32									worldLayer			= MAX_LAYER;	// common layer on the eMap::tileMap (can position renderBlock and TODO: filters collision)
	bool									isStatic			= true;			// if orthoOrigin ever changes at runtime, speeds up draw-order sorting
};

}      /* evil */
#endif /* EVIL_GAMEOBJECT_H */

