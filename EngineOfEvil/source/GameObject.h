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

#include "Component.h"
#include "HashIndex.h"
#include "Vector.h"

namespace evil { 

class eMap;
class eRenderTarget;

//*************************************************
//				eGameObject
// base composite-class for isolating eComponent groups
// and allowing meaningful intra- and inter-group communication
// handles the lifetime of all eComponent objects
//*************************************************
class eGameObject : public eClass {

	ECLASS_DECLARATION(eGameObject)

public:
	
	virtual									   ~eGameObject();
												eGameObject() = default;
												eGameObject( const eGameObject & other );
												eGameObject( eGameObject && other );
	eGameObject &								operator=( eGameObject other );

	virtual void								Think()										{}
	virtual void								DebugDraw( eRenderTarget * renderTarget )	{}

	void										UpdateComponents();	
	void										SetParent( eGameObject * newParent );
	eGameObject *								GetParent() const						{ return parent; }
	eGameObject *								GetFirstChild() const					{ return ( children.empty() ? nullptr : children.front() ); }


	eMap * const								GetMap()								{ return map; }
	const eVec2 &								GetOrigin()								{ return orthoOrigin; }
	void										SetOrigin( const eVec2 & newOrigin );
	Uint32										GetWorldLayer()							{ return worldLayer; }
	void										SetWorldLayer( Uint32 layer );
	void										SetWorldLayer( float zPosition );
	void										SetZPosition( float newZPosition );
	float										GetZPosition() const					{ return zPosition; }
	bool										IsStatic() const						{ return isStatic; }
	void										SetStatic( bool isStatic )				{ this->isStatic = isStatic; }
	bool										IsEnabled() const						{ return enabled; }
	void										Enable()								{ enabled = true; }
	void										Disable()								{ enabled = false; }

	template< class ComponentType, typename... Args >
	bool										AddComponent( eGameObject * owner, Args&&... params );

	template< class ComponentType >
	ComponentType *								GetComponent();

	template< class ComponentType >
	ComponentType * const						GetComponent() const;

	template< class ComponentType >
	std::vector< ComponentType * >				GetComponents();

	template< class ComponentType >
	std::vector< ComponentType * const>			GetComponents() const;

	template< class ComponentType >
	bool										RemoveComponent();

	template< class ComponentType >
	int											RemoveComponents();

private:

	void										UpdateComponentsOwner();

protected:

	eMap *										map;								// back-pointer to the eMap object owns *this
	
private:

	eHashIndex									componentsTypeHash;					// reduces component lookup time to average case O(1) using the types as hash keys
	std::vector<std::unique_ptr<eComponent>>	components;							// all eComponent-derived objects *this owns

	// parent and children lifetimes are handled by eMap::entities
	// FIXME/BUG: if *this is copied, then these have undefined behavior
	eGameObject *								parent;
	std::vector<eGameObject *>					children;

	eVec2										orthoOrigin;						// orthographic 2D global transfrom coordinates
	eVec2										relativeOrigin;						// position with respect to parent's orthoOrigin, if moving the parent, then this remains constant
	float										zPosition			= 0.0f;			// 3D position used for fluid renderBlock positioning (TODO: and other 3D related tasks)
	Uint32										worldLayer			= MAX_LAYER;	// common layer on the eMap::tileMap (can position renderBlock and TODO: filters collision)
	bool										isStatic			= true;			// if orthoOrigin ever changes at runtime, speeds up draw-order sorting
	bool										enabled				= true;			// if *this should be updated and/or drawn
};

//***************
// eGameObject::AddComponent
// perfect-forwards all params to the ComponentType constructor with the matching parameter list, then
// calls eComponent::Verify to check any user-defined pre-requesites for attaching the new component.
// If Verify returns true, then the new component is moved into the first empty element of the components vector.
// returns true on a successful add, false otherwise.
// DEBUG: be sure to compare the arguments of this fn to the desired constructor to avoid perfect-forwarding failure cases
// EG: deduced initializer lists, decl-only static const int members, 0|NULL instead of nullptr, overloaded fn names, and bitfields
//***************
template< class ComponentType, typename... Args >
bool eGameObject::AddComponent( eGameObject * owner, Args&&... params ) {
	auto & componentToAdd = std::make_unique< ComponentType >( owner, std::forward< Args >( params )... );

	if ( !componentToAdd->Verify() )
		return false;

	if ( components.empty() )
		componentsTypeHash.ClearAndResize( MAX_COMPONENTS );

	int index = 0;
	for ( auto && componentSlot : components ) {
		if ( componentSlot == nullptr ) {
			componentSlot = std::move( componentToAdd );
			break;
		} 
		++index;
	}

	componentsTypeHash.Add( ComponentType::Type, index );
	if ( index >= components.size() )
		components.emplace_back( std::move( componentToAdd ) );

	return true;
}

//***************
// eGameObject::GetComponent
// returns the most recently added component that matches the template type
// or that is derived from the template type in average case O(1) time
// EG: if the template type is eComponent, and components[0] type is eCollisionModel
// then components[0] will be returned because it derives from eComponent
//***************
template< class ComponentType >
ComponentType *	eGameObject::GetComponent() {
	for ( int index = componentsTypeHash.First( ComponentType::Type ); index != INVALID_ID; index = componentsTypeHash.Next( index ) ) {
		if ( components[ index ]->IsClassType( ComponentType::Type ) )
			return static_cast< ComponentType * >( components[ index ].get() );
	}

	return nullptr;
}


//***************
// eGameObject::GetComponent
// see non-const GetComponent<>
//***************
template< class ComponentType >
ComponentType * const	eGameObject::GetComponent() const {
	return GetComponent<ComponentType>();
}

//***************
// eGameObject::GetComponents
// returns a vector of pointers to the the requested component template type following the same match criteria as GetComponent
// DEBUG: the compiler has the option to copy-elide or move-construct componentsOfType into the return value here
//***************
template< class ComponentType >
std::vector< ComponentType * > eGameObject::GetComponents() {
	std::vector< ComponentType * > componentsOfType;

	for ( int index = componentsTypeHash.First( ComponentType::Type ); index != INVALID_ID; index = componentsTypeHash.Next( index ) ) {
		if ( components[ index ]->IsClassType( ComponentType::Type ) )
			componentsOfType.emplace_back( static_cast< ComponentType * >( components[ index ].get() ) );
	}

	return componentsOfType;
}

//***************
// eGameObject::GetComponents
// see non-const GetComponents
//***************
template< class ComponentType >
std::vector< ComponentType * const> eGameObject::GetComponents() const {
	std::vector< ComponentType * const> componentsOfType;

	for ( int index = componentsTypeHash.First( ComponentType::Type ); index != INVALID_ID; index = componentsTypeHash.Next( index ) ) {
		if ( components[ index ]->IsClassType( ComponentType::Type ) )
			componentsOfType.emplace_back( static_cast< ComponentType * const>( components[ index ].get() ) );
	}

	return componentsOfType;
}

//***************
// eGameObject::RemoveComponent
// sets the most recently added matching template type component pointer to nullptr,
// causing std::unique_ptr to delete the owned resource, and allowing
// another component to be constructed using the now-empty slot without
// modifying any other vector elements, size, or capacity
// returns true on successful removal
// returns false if components is empty, or no such component exists
//***************
template< class ComponentType >
bool eGameObject::RemoveComponent() {
	if ( components.empty() )
		return false;

	for ( int index = componentsTypeHash.First( ComponentType::Type ); index != INVALID_ID; index = componentsTypeHash.Next( index ) ) {
		if ( components[ index ]->IsClassType( ComponentType::Type ) ) {
			components[ index ] = nullptr;
			return true;
		}
	}

	return false;
}

//***************
// eGameObject::RemoveComponents
// returns the number of successful removals, or 0 if none are removed
//***************
template< class ComponentType >
int eGameObject::RemoveComponents() {
	if ( components.empty() )
		return 0;

	int numRemoved = 0;
	while ( RemoveComponent<ComponentType>() ) {
		++numRemoved;
	}

	return numRemoved;
}

}      /* evil */
#endif /* EVIL_GAMEOBJECT_H */

