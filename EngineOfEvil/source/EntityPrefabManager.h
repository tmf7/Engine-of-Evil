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
#ifndef EVIL_ENTITY_PREFAB_MANAGER_H
#define EVIL_ENTITY_PREFAB_MANAGER_H

#include "Entity.h"
#include "ResourceManager.h"

namespace evil {

//******************************************
//			eCreateEntityPrefabStrategy
// base class for setting how an eEntityPrefabManager
// instantiates prefab eEntity objects
// derived classes can contain different prefab instancing 
// cases in CreatePrefab
// param spawnArgs can be used to initialize the param newPrefab
// DEBUG: do not copy/move param spawnArgs into param newPrefab, eEntityPrefabManager moves it automatically
//******************************************
class eCreateEntityPrefabStrategy {
public:
					
	virtual													   ~eCreateEntityPrefabStrategy() = default;
																eCreateEntityPrefabStrategy() = default;
																eCreateEntityPrefabStrategy(const eCreateEntityPrefabStrategy & other) = default;
																eCreateEntityPrefabStrategy(eCreateEntityPrefabStrategy && other) = default;
	eCreateEntityPrefabStrategy &								operator=(const eCreateEntityPrefabStrategy & other) = default;
	eCreateEntityPrefabStrategy &								operator=(eCreateEntityPrefabStrategy && other) = default;

	virtual bool												CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) = 0;
};

//******************************************
//			eCreateEntityPrefabBasic
// default strategy for making eEntity base class instances
//******************************************
class eCreateEntityPrefabBasic : public eCreateEntityPrefabStrategy {
public:

	virtual bool												CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) override;
};


//******************************************
//			eEntityPrefabManager
// Handles all entity prefab allocation and freeing
// see also: eResourceManager template
//******************************************
class eEntityPrefabManager : public eResourceManager<eEntity> {

	ECLASS_DECLARATION(eEntityPrefabManager)

public:

	std::shared_ptr<eEntity> &									GetByShortName(const std::string & prefabShortName);
	const std::shared_ptr<eCreateEntityPrefabStrategy> &		GetCreatePrefabStrategy() const;
	void														SetCreatePrefabStrategy(const std::shared_ptr<eCreateEntityPrefabStrategy> & newStrategy);
	bool														SpawnInstance(eMap * onMap, const std::string & prefabShortName, const eVec3 & worldPosition);

	virtual bool												Init() override;
	virtual bool												LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) override;

private:

	bool														CreatePrefab(const char * sourceFilename, const std::string & prefabShortName, const eDictionary & spawnArgs, int & prefabManagerIndex);
	void														RegisterPrefab(const std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName);

private:

	eHashIndex													prefabShortNameHash;
	std::shared_ptr<eCreateEntityPrefabStrategy>				createPrefabStrategy = std::make_shared<eCreateEntityPrefabBasic>();
};

}      /* evil */
#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

