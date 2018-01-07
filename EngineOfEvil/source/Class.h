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
#ifndef EVIL_CLASS_H
#define EVIL_CLASS_H

#include "Definitions.h"

#define REGISTER_CLASS_TYPE(classname) const size_t classname::Type = std::hash<std::string>()(TO_STRING(classname))

namespace evil {

using ClassType_t = const std::size_t;

//*************************************************
//					eClass
// base class for all engine of evil classes that 
// can have instantiated objects and are designed 
// to be part of an inheritance chain beyond eClass
//*************************************************
class eClass{
public:

	virtual					   ~eClass() = default;
								eClass() = default;
								eClass(const eClass & other) = default;
								eClass(eClass && other) = default;
	eClass &					operator=(const eClass & other) = default;
	eClass &					operator=(eClass && other) = default;

	virtual bool				IsClassType(ClassType_t classType) const	{ return classType == Type; }

public:

	static ClassType_t			Type;

};

REGISTER_CLASS_TYPE(eClass);

}	   /* evil */
#endif /* EVIL_CLASS_H */
