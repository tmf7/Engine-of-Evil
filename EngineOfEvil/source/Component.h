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
#ifndef EVIL_COMPONENT_H
#define EVIL_COMPONENT_H

#include "Class.h"

//****************
// ECOMPONENT_DECLARATION
//
// This macro must be included in the declaration of any subclass of eComponent.
// It declares and defines the inline function for instantiating clones
// which maintain the run-time type when copying an eGameObject's eComponents.
//****************
#define ECOMPONENT_DECLARATION(componentsubclass)						\
virtual std::unique_ptr<eComponent> GetCopy() const override;			\

//****************
// ECOMPONENT_DEFINITION
// 
// This macro must be included in the eComponent subclass definition.
//****************
#define ECOMPONENT_DEFINITION(componentsubclass)						\
std::unique_ptr<eComponent> componentsubclass::GetCopy() const {		\
	return std::make_unique< componentsubclass >( *this );				\
}																		\

namespace evil {

class eGameObject;

//*************************************************
//				eComponent
// base class for all eGameObject behaviors
// eGameObjects handle the lifetime of eComponents
// DEBUG: always enusure the correct owner address is set
//*************************************************
class eComponent : public eClass {

	ECLASS_DECLARATION(eComponent)

private:

	friend class eGameObject;

public:

	virtual									   ~eComponent() = default;
												eComponent() = default;
												eComponent( const eComponent & other ) = default;
												eComponent( eComponent && other ) = default;
	eComponent &								operator=( const eComponent & other ) = default;
	eComponent &								operator=( eComponent && other ) = default;


	const eGameObject *							Owner() const								{ return owner; }
	eGameObject *								Owner()										{ return owner; }
	bool										IsEnabled() const							{ return enabled; }
	void										Enable()									{ enabled = true; }
	void										Disable()									{ enabled = false; }

												// checks any user-defined requirements before adding *this to the owner
	virtual bool								VerifyAdd() const							{ return true; }

	virtual void								SetOwner(eGameObject * newOwner)			{ owner = newOwner; }

												// called every frame before owner::Think in eGameObject::UpdateComponents
	virtual void								Update()									{}

												// used to maintain the runtime type of a derived eComponent when copying eGameObjects
	virtual std::unique_ptr<eComponent>			GetCopy() const								{ return std::make_unique< eComponent >( *this ); }

protected:

	eGameObject *								owner		= nullptr;			// back-pointer to user managing the lifetime of *this
	bool										enabled		= true;				// if *this should be Updated
};

}      /* evil */
#endif /* EVIL_COMPONENT_H */

