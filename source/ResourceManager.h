#ifndef EVIL_RESOURCE_MANAGER_H
#define EVIL_RESOURCE_MANAGER_H

#include "Definitions.h"
#include "HashIndex.h"
#include "Class.h"

//***************************************************************
//					eResourceManager
//
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
	virtual bool							BatchLoad(const char * resourceBatchFilename) = 0;
	virtual bool							Load(const char * resourceFilename) = 0;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<type> & result) = 0;
	virtual std::shared_ptr<type> &			Get(const char * resourceFilename) = 0;

	// no need to specialize these, but if needed do so in a derived class to avoid removing functionality
	// DEBUG: however, blocking visibility of the base class function can lead to undefined behavior (especially through base-class pointers/references)
	std::shared_ptr<type> &					Get(int resourceID)			{ return resourceList[resourceID]; }
	int										ResourceCount() const		{ return resourceList.size(); }
	void									Clear()						{ resourceList.clear(); resourceHash.Clear(); }

//	virtual int								GetClassType() const override = 0;

protected:

	std::vector<std::shared_ptr<type>>		resourceList;		// dynamically allocated resources
	eHashIndex								resourceHash;		// quick access to resourceList, indexed by resourceFilename
};

/*
	bool		LoadAndGetConstantText(TTF_Font * font, const char * text, const SDL_Color & color, std::shared_ptr<eImage> & result);


*/

#endif /* EVIL_RESOURCE_MANAGER_H */