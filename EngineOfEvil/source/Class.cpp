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
#include "Class.h"

using namespace evil;

/*
ClassType_t eClass::Type = std::hash<std::string>()(TO_STRING(eClass));

bool eClass::IsClassType(ClassType_t classType) const { 
	return classType == eClass::Type; 
}
*/

//**********************************************************************
//
//							eDynamicType
//
//**********************************************************************

static eDynamicType *			typeList = nullptr;		// head of a singly-linked list of all eClass-derived types
static eHierarchy<eDynamicType>	classHierarchy;			// top node/level of the hierarchy (nullptr parent)

//************************
// eDynamicType::eDynamicType
// links class definition into class hierarchy 
// DEBUG: this should only be called from CLASS_DECLARATION macro at startup,
// as eDynamicTypes are statically defined
// DEBUG: static variables can be initialized in any order, 
// so this handles the case that subclasses
// are initialized before superclasses.
//************************
eDynamicType::eDynamicType( const char * classname, const char * parentclassname, eClass *( *CreateInstance )(), void ( eClass::*Spawn )() )
	: classname(classname),
	  parentclassname(parentclassname),
	  CreateInstance(CreateInstance),
	  Spawn(Spawn),
	  typeNum(0),
	  lastChild(0),
	  initialized(false) {
	parentType		= eClass::GetTypeByClassname( parentclassname );		// DEBUG: nullptr if *this, as a derived-class, is constructed before its parent class

	// fixup parentTypes of any classes constructed before their parent classes
	for ( eDynamicType * type = typeList; type != nullptr; type = type->nextType ) {
		if ( ( type->parentType == nullptr ) && 
			 ( type->parentclassname == this->classname ) && 
			 ( type->classname != "eClass" ) ) {
			type->parentType = this;
		}
	}

	// insert *this into typeList alphabetically
	eDynamicType ** insert = nullptr;
	for ( insert = &typeList; *insert != nullptr; insert = &(*insert)->nextType ) {
		if ( classname < (*insert)->classname ) {
			nextType = *insert;
			*insert = this;
			break;
		}
	}

	if ( *insert == nullptr ) {
		*insert = this;
		nextType = nullptr;
	}
}

//************************
// eDynamicType::~eDynamicType
//************************
eDynamicType::~eDynamicType() {
	Shutdown();
}

//************************
// eDynamicType::Init
// adds this to the eHierarchy< eDynamicType > for later traversal
// and typeNum assignemt during eClass::Init,
// while tracking the number of children for each class
//************************
void eDynamicType::Init() {
	// already initialized by a subclass
	if ( initialized )
		return;

	// ensure the parent class is initialized first
	if ( parentType != nullptr && !parentType->initialized )
		parentType->Init();

	classNode.SetOwner( this );
	if ( parentType != nullptr )
		classNode.SetParent( parentType->classNode );
	else
		classNode.SetParent( classHierarchy );
	
	for( eDynamicType * type = parentType; type != nullptr; type = type->parentType )
		type->lastChild++;

	initialized = true;
}

//************************
// eDynamicType::Shutdown
// resets, but doesn't doesn't remove *this from the class types vectors 
// since the program should be shutting down when this is called.
//************************
void eDynamicType::Shutdown() {
	typeNum = 0;
	lastChild = 0;
}

//**********************************************************************
//
//							eClass
//
//**********************************************************************

std::vector<eDynamicType *>	eClass::typeNames;		// alphabetical order
std::vector<eDynamicType *>	eClass::typeNumbers;	// typeNum order
bool eClass::initialized	= false;


/*
//************************
// eClass::CallSpawn
//************************
void eClass::CallSpawn() {
	CallSpawnFunc( GetType() );
}

//************************
// eClass::CallSpawnFunc
//************************
eClass::classSpawnFunc_t eClass::CallSpawnFunc( eDynamicType * type ) {
	classSpawnFunc_t func;

	if ( type->parentType ) {
		func = CallSpawnFunc( type->parentType );
		if ( func == type->Spawn ) {
			// don't call the same fn twice in a row.
			// this can happen when subclasses don't have their own Spawn fn.
			return func;
		}
	}

	( this->*type->Spawn )();

	return type->Spawn;
}

//************************
// eClass::Spawn
//************************
void eClass::Spawn() {
}
*/

//************************
// eClass::Init
// call manually after all static eDynamicTypes are constructed.
// initializes all eDynamicTypes.
// DEBUG: only call this once at runtime.
//************************
void eClass::Init() {

	eErrorLogger::LogError( "Initializing class hierarchy\n", __FILE__, __LINE__ );

	if ( initialized ) {
		eErrorLogger::LogError( "...already initialized\n", __FILE__, __LINE__ );
		return;
	}

	eDynamicType *	type;
	for( type = typeList; type != nullptr; type = type->nextType ) {
		type->Init();
	}

	// types numbered via depth-first traversal of class hierarchy
	// allows for quick class|subclass lookup via int range comparisons
	int num = 0;
	for( type = classHierarchy.GetNext(); type != nullptr; type = type->classNode.GetNext(), num++ ) {
        type->typeNum = num;
		type->lastChild += num;
	}

	// initialize the alphabetical and typeNum ordered vectors
	// for lookup by name, and by number
	typeNames.resize( num );
	typeNumbers.resize( num );
	num = 0;
	for( type = typeList; type != nullptr; type = type->nextType, ++num ) {
		typeNames[ num ] = type;
		typeNumbers[ type->typeNum ] = type;
	}

	initialized = true;

	std::array<char, MAX_ESTRING_LENGTH> msgBuffer;
	snprintf( msgBuffer.data(), msgBuffer.size(), "...%i classes initialized.\n", typeNames.size() );
	eErrorLogger::LogError( msgBuffer.data(), __FILE__, __LINE__ );
}

//************************
// eClass::Shutdown
//************************
void eClass::Shutdown() {
	for( eDynamicType * type = typeList; type != nullptr; type = type->nextType ) {
		type->Shutdown();
	}

	typeNames.clear();
	typeNumbers.clear();
	initialized = false;
}

//************************
// eClass::GetTypeByClassname
// returns the eDynamicType for param classname This is a static function
// so it must be called as idClass::GetClass( classname )
//************************
eDynamicType * eClass::GetTypeByClassname( const char * classname ) {
	eDynamicType *	type;
	int				order;
	int				mid;
	int				min;
	int				max;
		
	if ( !initialized ) {		// eClass::Init hasn't been called yet, only check the types that have been constructed
		for( type = typeList; type != nullptr; type = type->nextType ) {
			if ( type->classname == classname )
				return type;
		}

	} else {
		// binary search of typeNames
		min = 0;
		max = typeNames.size() - 1;
		while ( min <= max ) {
			mid = ( min + max ) / 2;
			type = typeNames[ mid ];
			order = type->classname.compare( classname );

			if ( !order )
				return type;
			else if ( order > 0 )
				max = mid - 1;
			else
				min = mid + 1;
		}
	}

	return nullptr;
}

//************************
// eClass::GetTypeByNumber
//************************
eDynamicType * eClass::GetTypeByNumber( const int typeNum ) {
	if ( !initialized ) {		// eClass::Init hasn't been called yet, only check the types that have been constructed
		for( eDynamicType * type = typeList; type != nullptr; type = type->nextType ) {
			if ( type->typeNum == typeNum )
				return type;
		}

	} else if ( ( typeNum >= 0 ) && ( typeNum < typeNames.size() ) ) {
		return typeNumbers[ typeNum ];
	}

	return nullptr;
}

//************************
// eClass::GetClassname
//************************
const std::string & eClass::GetClassname() const {
	eDynamicType * type = GetType();
	return type->classname;
}

//************************
// eClass::GetParentClassname
//************************
const std::string & eClass::GetParentClassname() const {
	eDynamicType * type = GetType();
	return type->parentclassname;
}