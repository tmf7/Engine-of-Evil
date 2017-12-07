#include "EntityPrefabManager.h"
#include "Game.h"

//**************************
// eEntityPrefabManager::Init
//**************************
bool eEntityPrefabManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_PREFAB_ENTITIES);

	// register the error_prefab_entity as the first element of resourceList
	auto & errorPrefab = std::make_shared<eEntity>();
	errorPrefab->InitResource("error_prefab_entity", 0); 
	RegisterPrefab(errorPrefab);	// default error prefab entity
	return true;
}

//**************************
// eEntityPrefabManager::SetCreatePrefabStrategy
// assigns the method to be used for creating various
// eEntity-derived objects for later copying into the game-world
// via eEntityPrefabManager::SpawnInstance and/or eEntity::SpawnCopy
//**************************
void eEntityPrefabManager::SetCreatePrefabStrategy(const std::shared_ptr<eCreateEntityPrefabStrategy> & newStrategy) {
	createPrefabStrategy = newStrategy;
}

//**************************
// eEntityPrefabManager::GetCreatePrefabStrategy
// useful for re-using the same strategy across multiple eEntityPrefabManagers
// because eCreateEntityPrefabStrategy is stateless and requires no direct access to eEntityPrefabManager
//**************************
const std::shared_ptr<eCreateEntityPrefabStrategy> & eEntityPrefabManager::GetCreatePrefabStrategy() const {
	return createPrefabStrategy;
}

//**************************
// eEntityPrefabManager::LoadAndGet
// attempts to load the given .eprf file and
// returns true if the prefab was successfully loaded
// returns false otherwise
// DEBUG (.eprf file format):
// # first line comment\n
// # second line comment\n
// # any number of line comments above the definition\n
// prefabShortName: name\n				(shorthand name for easier EntityPrefabManager::CreatePrefab overriding)
// spriteFilename: filename.eimg\n		(leave spriteFilename, renderBlockSize, and imageColliderOffset empty if entity has no visuals)
// initialSpriteFrame: int\n			(which subframe of spriteFilename to start on, defaults to 0 if out-of-bounds, gets ignored if spriteFilename is blank)
// animationController: filename.ectrl	(spriteFilename and renderBlockSize must be set if animationController is set)
// renderBlockSize: x y z\n				(floats)
// renderImageOffset: x y\n				(floats)
// playerSelectable: [0|1]				(bool)
// isStatic: [0|1]\n					(bool)
// localBounds: xMin yMin xMax yMax\n	(floats, mins > maxs avoids allocating an eCollisionModel on the eEntity, collisionActive and movementSpeed will be ignored.)
// colliderOffset: x y\n
// movementSpeed: scalarValue\n			(float, set to 0 to avoid allocating an eMovementPlanner on the eEntity)
// collisionActive: [0|1]\n				(bool)
// (repeat, add any number of key-value string: string pairs to be copiend into eDictionary spawnArgs for use in CreatePrefab)
// [NOTE]: batch entity prefab files are .bprf
//**************************
bool eEntityPrefabManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) {
	if ((result = Get(resourceFilename))->IsValid())						// prefab already loaded
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) {														// unable to find/open file
		result = resourceList[0];											// default error default prefab entity
		return false;
	}

	eDictionary spawnArgs;
	char buffer[MAX_ESTRING_LENGTH];

	while (read.peek() == '#' || read.peek() == '\n')
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			// ignore comment or empty lines before prefab definition


	while (!read.eof()) {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ':');							// spawnArgs key
		std::string key(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		while (read.peek() == ' ' || read.peek() == '\t')					// skip whitespace b/t ':' and spawnArgs value
			read.ignore();
			
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '\n');							// spawnArgs value
		std::string value(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		if (!key.empty() && !value.empty())
			spawnArgs.Set(key.c_str(), value.c_str());
	}

	// add a prefab instance to the resourceList according to resourceFilename
	// and get the index where that prefab is located
	int prefabManagerIndex = 0;
	std::string prefabShortName = spawnArgs.GetString("prefabShortName", "error_prefab_entity");
	if (prefabShortName == "error_prefab_entity" || !CreatePrefab(resourceFilename, prefabShortName, spawnArgs, prefabManagerIndex)) {
		result = resourceList[0];
		return false;
	}	

	read.close();
	result = resourceList[prefabManagerIndex];
	return prefabManagerIndex > 0;
}


//***************
// eEntityPrefabManager::RegisterPrefab
// convenience function called after a std::make_shared<eEntity-type>(...) call
// in eEntityPrefabManager::CreatePrefab to ensure the resourceHash and resourceList are synchronized
// DEBUG: copies param newPrefab into the resourceList
//***************
void eEntityPrefabManager::RegisterPrefab(const std::shared_ptr<eEntity> & newPrefab) {
	resourceHash.Add(newPrefab->GetNameHash(), newPrefab->GetManagerIndex());
	resourceList.emplace_back(newPrefab);
}

//***************
// eEntityPrefabManager::CreatePrefab
// derived classes can add new prefab creation cases
// param spawnArgs can be used in this fn to initialize the instance
// or copy them into the instance 
// DEBUG: allows hash collisions when registering prefabs
// DEBUG: hooray a template-method design pattern that uses the Strategy design pattern
//***************
bool eEntityPrefabManager::CreatePrefab(const char * sourceFilename, const std::string & prefabShortName, const eDictionary & spawnArgs, int & prefabManagerIndex) {
	std::shared_ptr<eEntity> newPrefab = nullptr;
	prefabManagerIndex = resourceList.size();

	if (createPrefabStrategy->CreatePrefab(newPrefab, prefabShortName, spawnArgs) && newPrefab != nullptr) {
		newPrefab->spawnArgs = std::move(spawnArgs);
		newPrefab->InitResource(sourceFilename, prefabManagerIndex); 
		RegisterPrefab(newPrefab);
		return true;
	}

	return false;
}

//***************
// eEntityPrefabManager::SpawnInstance
// convenience function for duplicating entities a runtime
// calls the runtime type Spawn function of the prefab eEntity
// at param entityPrefabIndex within resourceList
//***************
bool eEntityPrefabManager::SpawnInstance(const int entityPrefabIndex, const eVec3 & worldPosition) {
	if (entityPrefabIndex < 0 || (size_t)entityPrefabIndex >= resourceList.size())
		return false;

	return Get(entityPrefabIndex)->SpawnCopy(worldPosition);
}

//***************
// eCreateEntityPrefabBasic::CreatePrefab
// derived classes can contain different prefab creation cases
// param spawnArgs can be used in this fn to initialize the instance
// DEBUG: do not copy/move spawnArgs into param newPrefab, eEntityPrefabManager moves them automatically after this fn 
//***************
bool eCreateEntityPrefabBasic::CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) {
	if (prefabShortName == "Entity")
		newPrefab = std::make_shared<eEntity>();
	else
		return false;

	bool success = true;
	success = newPrefab->AddRenderImage(	spawnArgs.GetString("spriteFilename", ""), 
											spawnArgs.GetVec3("renderBlockSize", "0 0 0"), 
											spawnArgs.GetInt("initialSpriteFrame", "0"), 
											spawnArgs.GetVec2("renderImageOffset", "0 0"), 
											spawnArgs.GetBool("playerSelectable", "0")
										);

	success = newPrefab->AddAnimationController(spawnArgs.GetString("animationController", ""));

	eQuat minMax = spawnArgs.GetVec4("localBounds", "1 1 0 0");					// default empty bounds
	eBounds localBounds(eVec2(minMax.x, minMax.y), eVec2(minMax.z, minMax.w));
	success = newPrefab->AddCollisionModel(localBounds, spawnArgs.GetVec2("colliderOffset", "0 0"), spawnArgs.GetBool("collisionActive", "0"));
	success = newPrefab->AddMovementPlanner(spawnArgs.GetFloat("movementSpeed", "0"));
	newPrefab->SetStatic(spawnArgs.GetBool("isStatic", "1"));
	return success;
}