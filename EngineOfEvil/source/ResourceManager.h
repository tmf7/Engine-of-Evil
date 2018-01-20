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
#ifndef EVIL_RESOURCE_MANAGER_H
#define EVIL_RESOURCE_MANAGER_H

#include "Definitions.h"
#include "HashIndex.h"
#include "Class.h"

namespace evil {

//***************************************************************
//					eResourceManager
// abstract template for resource allocation and freeing by type
// useful for initilizing and efficiently getting pointers to objects
// with properties that don't change beyond load-time initilization
// DEBUG: this template is designed to only be inhertied and overridden, not specialized
// DEBUG: use std::make_unique and std::make_shared to copy resources
// whose properties are intended to change during run-time (eg: Prefabricated objects)
//***************************************************************
template<class type>
class eResourceManager : public eClass {

	ECLASS_DECLARATION(eResourceManager<type>)

public:

	// NOTE: no primary template definition is provided to prevent that being used if an implicit template instantiation takes place before an explicit definition,
	// and these members are pure virtual to prevent creating an instance of the base template, as well as direct specialization (outside a specialized class declaration)
	// and to allow derived classes to override them, and still have access to protected data members
	virtual bool							Init() = 0;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<type> & result) = 0;

	// no need to specialize these, but if needed do so in a derived class to avoid removing functionality
	// DEBUG: however, obscuring visibility of the base class function can lead to undefined behavior (especially through base-class pointers/references)
	std::shared_ptr<type> &					GetByFilename(const char * resourceFilename);
	std::shared_ptr<type> &					GetByResourceID(int resourceID);
	bool									Load(const char * resourceFilename);
	bool									BatchLoad(const char * resourceBatchFilename);
	int										ResourceCount() const { return resourceList.size(); }
	void									Unload(int resourceID);
	void									Clear();

protected:

	std::vector<std::shared_ptr<type>>		resourceList;		// dynamically allocated resources
	eHashIndex								resourceHash;		// quick access to resourceList, indexed by resourceFilename
};

//***************************
// eResourceManager::Type (static member variable default initialization)
// DEBUG: ECLASS_DEFINITION(eClass, eResourceManager<type>) 
// needed two template<class type> attributes, so its expanded here
//***************************
template<class type>
ClassType_t eResourceManager<type>::Type = std::hash<std::string>()(TO_STRING(eResourceManager<type>));	

//***************************
// eResourceManager::IsClassType
// DEBUG: ECLASS_DEFINITION(eClass, eResourceManager<type>) 
// needed two template<class type> attributes, so its expanded here
//***************************
template<class type>
inline bool eResourceManager<type>::IsClassType( ClassType_t classType ) const {					
	if( classType == eResourceManager<type>::Type )											
		return true;															
	return eClass::IsClassType( classType );							
}

//***************************
// eResourceManager::Load
// convenience function, 
// same as LoadAndGet except 
// it only checks load success/failure
//***************************
template<class type>
inline bool eResourceManager<type>::Load(const char * resourceFilename)  {
	std::shared_ptr<type> tempResult = nullptr;
	return this->LoadAndGet(resourceFilename, tempResult);		// DEBUG: dereferencing "this" pointer to make it clear it's a polymorphic fn call to the derived class
}

//*************************
// eResourceManager::Clear
// clears all pointers to the current set 
// of resources, which allows them
// to be deleted once no object is using them,
// allows for new resources to load
// to optimize runtime memory usage
//*************************
template<class type>
inline void eResourceManager<type>::Clear() {
	resourceList.clear(); 
	resourceHash.Clear(); 
}

//*************************
// eResourceManager::Unload
// selective unloading of the resource
// with param resourceID
// DEBUG: user can compare resources used between levels/scenes
// to unload only those resources that are no longer needed
// to optimize runtime memory usage
// DEBUG: relies on items in resourceList to have the functions:
// int GetNameHash() const; and
// const std::string & GetSourceFilename() const;
//*************************
template<class type>
inline void eResourceManager<type>::Unload(int resourceID) {
	for ( int i = resourceHash.First( resourceList[ resourceID ]->GetNameHash() ); i != INVALID_ID; i = resourceHash.Next( i ) ) {
		if ( resourceList[ i ]->GetSourceFilename() == resourceList[ resourceID ]->GetSourceFilename() ) {
			resourceHash.RemoveIndex( resourceList[ resourceID ]->GetNameHash(), resourceID );
			break;
		}
	}
	resourceList.erase( resourceID );
}

//*************************
// eResourceManager::GetByResourceID
// returns the resource with the given resourceID
// DEBUG: no range checking occurs
//*************************
template<class type>
inline std::shared_ptr<type> & eResourceManager<type>::GetByResourceID(int resourceID) {
	return resourceList[resourceID];  
}

//***************************
// eResourceManager::GetByFilename
// returns a resource pointer if it exists
// if param resourceFilename is null or the resource doesn't exist
// then it returns the default error resource pointer
// DEBUG: relies on items in resourceList to have the function:
// const std::string & GetSourceFilename() const;
//***************************
template<class type>
inline std::shared_ptr<type> & eResourceManager<type>::GetByFilename(const char * resourceFilename) {
	if (!resourceFilename) 
		return resourceList[0]; // default error resource

	int hashKey = resourceHash.GetHashKey(std::string(resourceFilename));
	for (int i = resourceHash.First(hashKey); i != -1; i = resourceHash.Next(i)) {
		if (resourceList[i]->GetSourceFilename() == resourceFilename)
			return resourceList[i];
	}

	return resourceList[0];
}

//***************************
// eResourceManager::BatchLoad
// loads a batch of resource files
// user can optionally call Clear() or Unload()
// prior to this to facilitate starting with a fresh set of resources
// DEBUG (batch file format):
// resourceFilename\n
// resourceFilename\n
// (repeat)
//***************************
template<class type>
inline bool eResourceManager<type>::BatchLoad(const char * resourceBatchFilename) {
	std::ifstream	read(resourceBatchFilename);
	std::string message;
	int numLoadFailures = 0;
	
	// unable to find/open file
	if(!read.good()) {
			message = "Unable to open batch file: ";
			message += resourceBatchFilename;
			eErrorLogger::LogError(message.c_str(), __FILE__, __LINE__);
	}

	char resourceFilename[MAX_ESTRING_LENGTH];
	while (!read.eof()) {
		read >> resourceFilename;
		if (!VerifyRead(read)) {
			message = "Batch file read failure: ";
			message += resourceBatchFilename;
			message += "\nLast read: ";
			message += resourceFilename;
			eErrorLogger::LogError(message.c_str(), __FILE__, __LINE__);
		}

		if (!Load(resourceFilename)) {
			message = "Resource load failure: ";
			message += resourceFilename;
			eErrorLogger::LogError(message.c_str(), __FILE__, __LINE__);
			++numLoadFailures;
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();

	if (numLoadFailures > 0) {
		message = resourceBatchFilename;
		message += " batch file load failures: ";
		message += std::to_string(numLoadFailures);
		eErrorLogger::LogError(message.c_str(), __FILE__, __LINE__);
	}

	return numLoadFailures == 0;
}

}      /* evil */
#endif /* EVIL_RESOURCE_MANAGER_H */