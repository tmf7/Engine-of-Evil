#ifndef EVIL_ENTITY_PREFAB_MANAGER_H
#define EVIL_ENTITY_PREFAB_MANAGER_H

#include "Entity.h"
#include "ResourceManager.h"


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
public:

	const std::shared_ptr<eCreateEntityPrefabStrategy> &		GetCreatePrefabStrategy() const;
	void														SetCreatePrefabStrategy(const std::shared_ptr<eCreateEntityPrefabStrategy> & newStrategy);
	bool														SpawnInstance(const int entityPrefabIndex, const eVec3 & worldPosition);

	virtual bool												Init() override;
	virtual bool												LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) override;

	virtual int													GetClassType() const override				{ return CLASS_ENTITYPREFAB_MANAGER; }
	virtual bool												IsClassType(int classType) const override	{ 
																	if(classType == CLASS_ENTITYPREFAB_MANAGER) 
																		return true; 
																	return eResourceManager<eEntity>::IsClassType(classType); 
																}

private:

	bool														CreatePrefab(const char * sourceFilename, const std::string & prefabShortName, const eDictionary & spawnArgs, int & prefabManagerIndex);
	void														RegisterPrefab(const std::shared_ptr<eEntity> & newPrefab);

private:

	std::shared_ptr<eCreateEntityPrefabStrategy>				createPrefabStrategy = std::make_shared<eCreateEntityPrefabBasic>();
};

#endif /* EVIL_ENTITY_PREFAB_MANAGER_H */

