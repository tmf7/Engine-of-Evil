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
										eResource(const std::string & sourceFilename, int managerIndex)
											: sourceFilename(sourceFilename),
											  managerIndex(managerIndex) {
											nameHash = std::hash<std::string>()(sourceFilename);
										}

	eResource &							operator=(const eResource & other) = default;
	eResource &							operator=(eResource && other) = default;

	const std::string &					GetSourceFilename() const	{ return sourceFilename; }
	int									GetNameHash() const			{ return nameHash; }
	int									GetManagerIndex() const		{ return managerIndex; }
	bool								IsValid() const				{ return managerIndex > 0; }

protected:

	std::string							sourceFilename;
	int									nameHash;
	int									managerIndex;
};

#endif  /* EVIL_RESOURCE_H */