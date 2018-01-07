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

namespace evil { namespace components {

class eGameObject;


//*************************************************
//				eComponent
// base class for all in-game interactive objects
// eGameObjects handle the lifetime of eComponents
//*************************************************
class eComponent : public eClass {
private:

	friend class eGameObject;

public:

	const eGameObject *							Owner() const								{ return owner; }
	eGameObject *								Owner()										{ return owner; }

	virtual bool								IsClassType( ClassType_t classType ) const override	{ 
													if( classType == Type ) 
														return true; 
													return eClass::IsClassType( classType ); 
												}

protected:

												// safety-reminder, disallow derived classes from instantiation without an owner
												eComponent() = default;

												
												// called every frame before owner::Think in eGameObject::UpdateComponents
	virtual void								Update()									{}

												// used to maintain the runtime type of a derived eComponent when copying eGameObjects
	virtual std::unique_ptr<eComponent>			GetCopy() const								{ return std::make_unique< eComponent >( *this ); }

	virtual void								SetOwner(eGameObject * newOwner)			{ owner = newOwner; }

public:

	static ClassType_t							Type;

protected:

	eGameObject *								owner = nullptr;			// back-pointer to user managing the lifetime of *this
};

REGISTER_CLASS_TYPE(eComponent);

} }    /* evil::components */
#endif /* EVIL_COMPONENT_H */

