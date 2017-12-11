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