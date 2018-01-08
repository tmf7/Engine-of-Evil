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
	
	virtual								   ~eGameObject() = default;
											eGameObject() = default;
											eGameObject( const eGameObject & other );
											eGameObject( eGameObject && other );
	eGameObject &							operator=( eGameObject other );


	virtual void							Think()										{}
	virtual void							DebugDraw( eRenderTarget * renderTarget )	{}

	void									UpdateComponents();	
	eMap * const							GetMap()								{ return map; }
	const eVec2 &							GetOrigin()								{ return orthoOrigin; }
	void									SetOrigin( const eVec2 & newOrigin )	{ orthoOrigin = newOrigin; }
	Uint32									GetWorldLayer()							{ return worldLayer; }
	void									SetWorldLayer( Uint32 layer );
	void									SetWorldLayer( float zPosition );
	void									SetZPosition( float newZPosition );
	float									GetZPosition() const					{ return zPosition; }
	bool									IsStatic() const						{ return isStatic; }
	void									SetStatic( bool isStatic )				{ this->isStatic = isStatic; }

// FREEHILL BEGIN generic component test
	template< class ComponentType, typename... Args >
	void									AddComponent(eGameObject * owner, Args&&... params );

	template< class ComponentType >
	ComponentType &							GetComponent();

	template< class ComponentType >
	bool									RemoveComponent();

	template< class ComponentType >
	std::vector< ComponentType * >			GetComponents();

	template< class ComponentType >
	int										RemoveComponents();

/*
	// GetComponent (const)
	// GetComponents (const)
	// TODO: ??? something ???
*/
// FREEHILL END generic component test

private:

	void									UpdateComponentsOwner();

protected:

	std::vector<std::unique_ptr<eComponent>> components;

	eMap *									map;								// back-pointer to the eMap object owns *this
	
private:

	eVec2									orthoOrigin;						// orthographic 2D global transfrom coordinates
	float									zPosition			= 0.0f;			// 3D position used for fluid renderBlock positioning (TODO: and other 3D related tasks)
	Uint32									worldLayer			= MAX_LAYER;	// common layer on the eMap::tileMap (can position renderBlock and TODO: filters collision)
	bool									isStatic			= true;			// if orthoOrigin ever changes at runtime, speeds up draw-order sorting
};

// FREEHILL BEGIN generic component test

//***************
// eGameObject::AddComponent
// perfect-forwards all params to the ComponentType constructor with the matching parameter list
// DEBUG: be sure to compare the arguments of this fn to the desired constructor to avoid perfect-forwarding failure cases
// EG: deduced initializer lists, decl-only static const int members, 0|NULL instead of nullptr, overloaded fn names, and bitfields
//***************
template< class ComponentType, typename... Args >
void eGameObject::AddComponent(eGameObject * owner, Args&&... params ) {
	components.emplace_back( std::make_unique< ComponentType >(owner, std::forward< Args >( params )... ) );
}

//***************
// eGameObject::GetComponent
// returns the first component that matches the template type
// or that is derived from the template type
// EG: if the template type is Component, and components[0] type is BoxCollider
// then components[0] will be returned because it derives from Component
//***************
template< class ComponentType >
ComponentType &	eGameObject::GetComponent() {
	for ( auto && component : components ) {
		if ( component->IsClassType( ComponentType::Type ) )
			return *static_cast< ComponentType * >( component.get() );
	}

	return *std::unique_ptr< ComponentType >( nullptr );
}

//***************
// eGameObject::RemoveComponent
// returns true on successful removal
// returns false if components is empty, or no such component exists
//***************
template< class ComponentType >
bool eGameObject::RemoveComponent() {
	if ( components.empty() )
		return false;

	auto & index = std::find_if( components.begin(), 
									components.end(), 
									[ classType = ComponentType::Type ]( auto & component ) { 
									return component->IsClassType( classType ); 
									} );

	bool success = index != components.end();

	if ( success )
		components.erase( index );

	return success;
}

//***************
// eGameObject::GetComponents
// returns a vector of pointers to the the requested component template type following the same match criteria as GetComponent
// DEGUG: the compiler has the option to copy-elide or move-construct componentsOfType into the return value here
// TODO: pass in the number of elements desired (eg: up to 7, or only the first 2) which would allow a std::array return value,
// except there'd need to be a separate fn for getting them *all* if the user doesn't know how many such Components the GameObject has
// TODO: define a GetComponentAt<ComponentType, int>() that can directly grab up to the the n-th component of the requested type
//***************
template< class ComponentType >
std::vector< ComponentType * > eGameObject::GetComponents() {
	std::vector< ComponentType * > componentsOfType;

	for ( auto && component : components ) {
		if ( component->IsClassType( ComponentType::Type ) )
			componentsOfType.emplace_back( static_cast< ComponentType * >( component.get() ) );
	}

	return componentsOfType;
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
	bool success = false;

	do {
		auto & index = std::find_if( components.begin(), 
										components.end(), 
										[ classType = ComponentType::Type ]( auto & component ) { 
										return component->IsClassType( classType ); 
										} );

		success = index != components.end();

		if ( success ) {
			components.erase( index );
			++numRemoved;
		}
	} while ( success );

	return numRemoved;
}

// FREEHILL END generic component test

}      /* evil */
#endif /* EVIL_GAMEOBJECT_H */

