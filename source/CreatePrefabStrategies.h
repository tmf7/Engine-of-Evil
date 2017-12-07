#ifndef EVIL_CREATE_PREFAB_STRATEGIES_H
#define EVIL_CREATE_PREFAB_STRATEGIES_H

#include "EntityPrefabManager.h"

//******************************************
//			eCreateEntityPrefabUser
// default strategy for making eEntity base class instances
//******************************************
class eCreateEntityPrefabUser: public eCreateEntityPrefabStrategy {
public:

	virtual bool	CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) override;
};

#endif /* EVIL_CREATE_PREFAB_STRATEGIES_H */