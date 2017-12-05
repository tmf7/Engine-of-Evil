#ifndef EVIL_ENTITY_PREFAB_MANAGER_H
#define EVIL_ENTITY_PREFAB_MANAGER_H

#include "Entity.h"
#include "ResourceManager.h"

//******************************************
//			eEntityPrefabManager
// Handles all entity prefab allocation and freeing
// see also: eResourceManager template
//******************************************
class eEntityPrefabManager : public eResourceManager<eEntity> {
public:

	bool									SpawnInstance(const int entityPrefabIndex, const eVec3 & worldPosition);

	virtual bool							Init() override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) override;
	virtual bool							CreatePrefab(const char * sourceFilename, const std::string & prefabShortName, const eDictionary & spawnArgs, int & prefabManagerIndex);

	virtual int								GetClassType() const override				{ return CLASS_ENTITYPREFAB_MANAGER; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_ENTITYPREFAB_MANAGER) 
													return true; 
												return eResourceManager<eEntity>::IsClassType(classType); 
											}
protected:

	void									RegisterPrefab(const std::shared_ptr<eEntity> & newPrefab);
};

#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

