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
#ifndef EVIL_SHERO_H
#define EVIL_SHERO_H

#include "Entity.h"

namespace logic {

	using namespace evil;

class sHero : public eEntity {

	ECLASS_DECLARATION(sHero)

public:

	virtual bool						SpawnCopy( eMap * onMap, const eVec3 & worldPosition ) override;

	virtual void						Think() override;

private:

	// DEBUG: both of these reference members have lifetimes dictated by their eGameObject owner (ie: *this)
	// DEBUG: this aggregation prevents the compiler from generating an assignment operator
	// TODO: if needed, write the assignment operator to perform new GetComponent<> on the new object's reference members
	eCollisionModel &					collisionModel;
	eAnimationController &				animationController;

	eVec2								oldFacingDirection			= vec2_oneZero;
	const int							xSpeedParameterHash		= std::hash< std::string >()( "xSpeed" );
	const int							ySpeedParameterHash		= std::hash< std::string >()( "ySpeed" );
	const int							magnitudeParameterHash	= std::hash< std::string >()( "magnitude" );
};

}	   /* evil */
#endif /* EVIL_SHERO_H */