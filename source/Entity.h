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