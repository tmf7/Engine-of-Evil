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
#ifndef ENTITY_H
#define ENTITY_H

#include "GameObject.h"
#include "Resource.h"
#include "Dictionary.h"

//*************************************************
//					eEntity
// objects that dynamically interact with the game environment
//*************************************************
class eEntity : public eGameObject, public eResource {
public:

	friend class eGame;						// for access to assign spawnedEntityID and spawnName
	friend class eEntityPrefabManager;		// for access to spawnArgs

public:

	void								SetPlayerSelected(bool isSelected);
	bool								GetPlayerSelected() const;
	const eDictionary &					GetSpawnArgs() const;
	const std::string &					SpawnName() const;
	int									SpawnID() const;

	virtual bool						SpawnCopy(const eVec3 & worldPosition);
	virtual void						DebugDraw() override;

	virtual int							GetClassType() const override					{ return CLASS_ENTITY; }
	virtual bool						IsClassType(int classType) const override		{ 
											if(classType == CLASS_ENTITY) 
												return true; 
											return eGameObject::IsClassType(classType);
										}

private:

	eDictionary							spawnArgs;				// populated during eEntityPrefabManager::CreatePrefab, used for initialization in eCreateEntityPrefabStrategy::CreatePrefab-overridden methods
	std::string							spawnName;				// unique name for this instance (eg: "prefabShortName_spawnedEntityID")
	int									spawnedEntityID;		// index within eGame::entities
	bool								playerSelected;			// player is controlling this eEntity


};

//**************
// eEntity::SetPlayerSelected
//**************
inline void eEntity::SetPlayerSelected(bool isSelected) {
	playerSelected = isSelected;
}

//**************
// eEntity::GetPlayerSelected
//**************
inline bool eEntity::GetPlayerSelected() const {
	return playerSelected;
}

//**************
// eEntity::GetSpawnArgs
//**************
inline const eDictionary & eEntity::GetSpawnArgs() const {
	return spawnArgs;
}

//**************
// eEntity::SpawnID
//**************
inline int eEntity::SpawnID() const {
	return spawnedEntityID;
}

//**************
// eEntity::SpawnName
// unique instance name
//**************
inline const std::string &	eEntity::SpawnName() const {
	return spawnName;
}

#endif /* ENTITY_H */