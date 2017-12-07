#ifndef EVIL_RESOURCE_H
#define EVIL_RESOURCE_H

#include <string>
#include <functional>

//*************************
//		eResource
// base class for resources managed
// by eResourceManager classes
// publicly inherit from this to ensure
// the eResourceManager can fully
// access/manipulate its resource
//*************************
class eResource {
public:

	virtual							   ~eResource() = default;
										eResource() = default;
										eResource(const eResource & other) = default;
										eResource(eResource && other) = default;
										eResource(const char * sourceFilename, int managerIndex) { InitResource(sourceFilename, managerIndex); }

	eResource &							operator=(const eResource & other) = default;
	eResource &							operator=(eResource && other) = default;

	const std::string &					GetSourceFilename() const	{ return sourceFilename; }
	int									GetNameHash() const			{ return nameHash; }
	int									GetManagerIndex() const		{ return managerIndex; }
	bool								IsValid() const				{ return managerIndex > 0; }

protected:

										// for post-load initialization (eg: see eEntityPrefabManager::CreatePrefab)
	void								InitResource(const char * sourceFilename, int managerIndex) {
											this->sourceFilename = sourceFilename;
											this->managerIndex = managerIndex;
											nameHash = std::hash<std::string>()(sourceFilename);

										}

protected:

	std::string							sourceFilename		= "not_managed";
	int									nameHash			= -1;	
	int									managerIndex		= -1;
};

#endif  /* EVIL_RESOURCE_H */