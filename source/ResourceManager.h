#ifndef EVIL_RESOURCE_MANAGER_H
#define EVIL_RESOURCE_MANAGER_H

#include "Definitions.h"
#include "HashIndex.h"
#include "Class.h"

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
public:

	// NOTE: no primary template definition is provided to prevent that being used if an implicit template instantiation takes place before an explicit definition,
	// and these members are pure virtual to prevent creating an instance of the base template, as well as direct specialization (outside a specialized class declaration)
	// and to allow derived classes to override them, and still have access to protected data members
	virtual bool							Init() = 0;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<type> & result) = 0;

	virtual int								GetClassType() const override = 0;

	// no need to specialize these, but if needed do so in a derived class to avoid removing functionality
	// DEBUG: however, obscuring visibility of the base class function can lead to undefined behavior (especially through base-class pointers/references)
	std::shared_ptr<type> &					Get(const char * resourceFilename);
	std::shared_ptr<type> &					Get(int resourceID);
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
// DEBUG: relies on items in resourceList to have the function:
// const std::string & GetNameHash() const;
//*************************
template<class type>
inline void eResourceManager<type>::Unload(int resourceID) {
	resourceList.erase(resourceID);
	resourceHash.RemoveIndex(resourceList[resourceID]->GetNameHash(), resourceID);
}

//*************************
// eResourceManager::Get
// returns the resource with the given resourceID
// DEBUG: no range checking occurs
//*************************
template<class type>
inline std::shared_ptr<type> & eResourceManager<type>::Get(int resourceID) {
	return resourceList[resourceID];  
}

//***************************
// eResourceManager::Get
// returns an resource pointer if it exists
// if param resourceFilename is null or the resource doesn't exist
// then it returns the default error resource pointer
// DEBUG: relies on items in resourceList to have the function:
// const std::string & GetSourceFilename() const;
//***************************
template<class type>
inline std::shared_ptr<type> & eResourceManager<type>::Get(const char * resourceFilename) {
	if (!resourceFilename) 
		return resourceList[0]; // default error resource

	// search for pre-existing texture
	int hashKey = resourceHash.GetHashKey(std::string(resourceFilename));
	for (int i = resourceHash.First(hashKey); i != -1; i = resourceHash.Next(i)) {
		if (resourceList[i]->GetSourceFilename() == resourceFilename)
			return resourceList[i];
	}
	return resourceList[0];		// default error resource
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
	
	// unable to find/open file
	if(!read.good())
		return false;

	char resourceFilename[MAX_ESTRING_LENGTH];
	while (!read.eof()) {
		read >> resourceFilename;
		if (!VerifyRead(read))
			return false;

		if (!Load(resourceFilename))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

#endif /* EVIL_RESOURCE_MANAGER_H */