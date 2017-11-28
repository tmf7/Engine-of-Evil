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

	virtual bool							Load(const char * resourceFilename) override;
	virtual bool							Init() override;
	virtual bool							BatchLoad(const char * resourceBatchFilename) override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) override;

	virtual int								GetClassType() const override { return CLASS_ENTITYPREFAB_MANAGER; }
};

#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

