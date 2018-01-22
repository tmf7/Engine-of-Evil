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
#include "Hierarchy.h"

//********************************************************************************
// ECLASS_DECLARATION
// This macro must be included in the declaration of any subclass of eClass.
// It declares variables used in type checking.
//********************************************************************************
#define ECLASS_DECLARATION( classname )											\
public:																			\
	static std::unique_ptr<eClass>			CreateInstance();					\
	static eDynamicType						Type;								\
	virtual eDynamicType *					GetType() const;


//********************************************************************************
// ECLASS_DEFINITION
// This macro must be included in the class definition to properly initialize 
// variables used in type checking. Take special care to ensure that the 
// proper parentclass is indicated or the run-time type information will be incorrect.
//********************************************************************************
#define ECLASS_DEFINITION( parentclassname, classname )							\
eDynamicType classname::Type( #classname,										\
							  #parentclassname,									\
							  &classname::CreateInstance,						\
							  ( void ( eClass::* )() )&classname::Spawn );		\
																				\
std::unique_ptr<eClass> classname::CreateInstance() {							\
	return std::make_unique<classname>();										\
}																				\
																				\
eDynamicType * classname::GetType() const {										\
	return &classname::Type;													\
}

namespace evil {

class eClass;

//*****************************
//		eDynamicType
// defines  runtime type information
// used by the Engine Of Evil class hierarchy
//*****************************
class eDynamicType {
public:

								eDynamicType( const char *classname, const char * parentclassname, eClass *( *CreateInstance )(), void ( eClass::*Spawn )() );
							   ~eDynamicType();

	void						Init();
	void						Shutdown();
	bool						IsType( const eDynamicType & classType ) const;

public:

	eHierarchy<eDynamicType>	classNode;
	std::string					classname;
	std::string					parentclassname;
	eClass *					( *CreateInstance )();
	void						( eClass::*Spawn )();

	eDynamicType *				parentType;
	eDynamicType *				nextType;			// for sorting in alphabetical order at startup
	int							typeNum;
	int							lastChild;
	bool						initialized;

};

//*************************************************
//					eClass
// base class for all Engine of Evil game objects
//*************************************************
class eClass {

	ECLASS_DECLARATION( eClass )

public:

	virtual							       ~eClass() = default;
											eClass() = default;
											eClass(const eClass & other) = default;
											eClass(eClass && other) = default;
	eClass &								operator=(const eClass & other) = default;
	eClass &								operator=(eClass && other) = default;


//	void									Spawn();
//	void									CallSpawn();

	bool									IsType( const eDynamicType & classtype ) const;
	const std::string &						GetClassname() const;
	const std::string &						GetParentClassname() const;


	// Static functions
	static void								Init();
	static void								Shutdown();
	static eDynamicType *					GetTypeByClassname( const char * classname );
	static eDynamicType *					GetTypeByNumber( int num );
	static int								GetNumTypes() { return typeNames.size(); }

private:

//	using classSpawnFunc_t = void (eClass::*)();

//	classSpawnFunc_t						CallSpawnFunc( eDynamicType * cls );

	static std::vector<eDynamicType *>		typeNumbers;		// alphabetical order
	static std::vector<eDynamicType *>		typeNames;			// typeNum order
	static bool								initialized;
};

//***********************
// eDynamicType::IsType
// returns true if the object's class 
// is derived from the class defined by param classType
//***********************
inline bool eDynamicType::IsType( const eDynamicType & classType ) const {
	return ( ( typeNum >= classType.typeNum ) && ( typeNum <= classType.lastChild ) );
}

//***********************
// eClass::IsType
// see eDynamicType::IsType
//***********************
inline bool eClass::IsType( const eDynamicType & classType ) const {
	eDynamicType * subClass = GetType();
	return subClass->IsType( classType );
}

}	   /* evil */
#endif /* EVIL_CLASS_H */
