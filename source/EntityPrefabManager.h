#ifndef EVIL_ENTITY_PREFAB_MANAGER_H
#define EVIL_ENTITY_PREFAB_MANAGER_H

#include "Definitions.h"
#include "HashIndex.h"
#include "Entity.h"

//**********************************
//			eEntityPrefabManager
// Handles all entity prefab allocation and freeing
// DEBUG: --no other object/system should allocate/free entity prefabs--
//**********************************
class eEntityPrefabManager : public eClass {
public:

	bool			Init();
	bool			BatchLoadPrefabs(const char * prefabBatchFile);
	bool			GetPrefab(const char * filename, std::shared_ptr<eEntity> & result);
	bool			GetPrefab(int preafabID, std::shared_ptr<eEntity> & result);
	bool			LoadPrefab(const char * filename, std::shared_ptr<eEntity> & result);
	int				GetNumPrefabs() const;
	void			Clear();

	virtual int		GetClassType() const override { return CLASS_ENTITYPREFABMANAGER; }

private:

	std::vector<std::shared_ptr<eEntity>>		prefabList;			// dynamically allocated entity prefab resources
	eHashIndex									prefabFilenameHash;	// quick access to prefabList
};

//***************
// eEnityPrefabManager::GetNumPrefabs
//***************
inline int eEntityPrefabManager::GetNumPrefabs() const {
	return prefabList.size();
}

#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

