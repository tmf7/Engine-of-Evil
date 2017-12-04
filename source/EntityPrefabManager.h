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

	virtual bool							Init() override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) override;

	bool									SpawnInstance(const int entityPrefabIndex, const eVec3 & worldPosition);

	virtual int								GetClassType() const override				{ return CLASS_ENTITYPREFAB_MANAGER; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_ENTITYPREFAB_MANAGER) 
													return true; 
												return eResourceManager<eEntity>::IsClassType(classType); 
											}
};

#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

