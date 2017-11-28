#include "Game.h"

//**************************
// eEntityPrefabManager::Init
// TODO: output to an error log file upon failure
//**************************
bool eEntityPrefabManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_IMAGES);

	// register the error_prefab_entity as the first element of prefabList
	entitySpawnArgs_t defaultSpawnArgs;
	int hashKey = resourceHash.GetHashKey(std::string("error_prefab_entity"));
	resourceHash.Add(hashKey, resourceList.size());
	try {
		resourceList.emplace_back(std::make_shared<eEntity>(defaultSpawnArgs));	// error prefab
	} catch (const badEntityCtorException & e) {
		std::string message = e.what + " caused eEntity (0) prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
	return true;
}

//**************************
// eEntityPrefabManager::BatchLoad
// loads a batch of entity prefab resources
// user can optionally call Clear() or Unload()
// prior to this to facilitate starting with a fresh set of prefabs
// DEBUG (.bepf file format):
// 1=prefabFilename\n	(prefabList index used when spawning eEntities) (everything past the first string is ignored)
// 2=prefabFilename\n	(the 0th index is the error eEntity)
// (repeat)
//**************************
bool eEntityPrefabManager::BatchLoad(const char * resourceBatchFilename) {
	std::ifstream	read(resourceBatchFilename);

	// unable to find/open file
	if(!read.good())
		return false;

	char resourceFilename[MAX_ESTRING_LENGTH];
	while (!read.eof()) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// skip prefabList index comment
		read >> resourceFilename;
		if (!VerifyRead(read))
			return false;

		if (!Load(resourceFilename))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//**************************
// eEntityPrefabManager::LoadAndGet
// attempts to load the given .eprf file and
// returns true if the prefab was successfully loaded
// returns false otherwise
// DEBUG (.eprf file format):
// # first line comment\n 
// spriteFilename=filename.png\n		(leave spriteFilename, renderBlockSize, and imageColliderOffset empty if entity has no visuals)
// renderBlockSize= x y z\n				(floats)
// imageCollisionOffset= x y\n			(floats)
// localBounds= xMin yMin xMax yMax\n	(floats, mins = -maxs avoids allocating an eCollisionModel on the eEntity, collisionActive and movementSpeed will be ignored.)
// movementSpeed= scalarValue\n			(float, set to 0 to avoid allocating an eMovementPlanner on the eEntity)
// collisionActive= trueOrFalseHere\n	(boolalpha)
//**************************
bool eEntityPrefabManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) {

	// prefab already loaded
	if ((result = Get(resourceFilename)) != resourceList[0])
		return true;

	std::ifstream	read(resourceFilename);
	// unable to find/open file
	if (!read.good()) 
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip first line comment

	entitySpawnArgs_t spawnArgs;
	spawnArgs.sourceFilename = resourceFilename;

	char spriteFilename[MAX_ESTRING_LENGTH];
	read.ignore(std::numeric_limits<std::streamsize>::max(), '=');		// spriteFilename
	memset(spriteFilename, 0, sizeof(spriteFilename));
	read.getline(spriteFilename, sizeof(spriteFilename), '\n');
	if (!VerifyRead(read))
		return false;

	spawnArgs.spriteFilename = spriteFilename;
	if (!spawnArgs.spriteFilename.empty()) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// renderBlockSize
		read >> spawnArgs.renderBlockSize.x;
		read >> spawnArgs.renderBlockSize.y;
		read >> spawnArgs.renderBlockSize.z;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// imageCollisionOffset
		read >> spawnArgs.imageColliderOffset.x;
		read >> spawnArgs.imageColliderOffset.y;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read))
			return false;

	} else {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	read.ignore(std::numeric_limits<std::streamsize>::max(), '=');		// localBounds
	read >> spawnArgs.localBounds[0][0];
	read >> spawnArgs.localBounds[0][1];
	read >> spawnArgs.localBounds[1][0];
	read >> spawnArgs.localBounds[1][1];
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	if (!VerifyRead(read))
		return false;

	if (!spawnArgs.localBounds.IsEmpty()) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// movementSpeed
		read >> spawnArgs.movementSpeed;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// collisionActive
		read >> std::boolalpha >> spawnArgs.collisionActive;
//		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read))
			return false;
	}
	read.close();
	spawnArgs.prefabManagerIndex = resourceList.size();

	// register the requested entity prefab
	int hashKey = resourceHash.GetHashKey(std::string(resourceFilename));
	resourceHash.Add(hashKey, resourceList.size());
	try {
		result = std::make_shared<eEntity>(spawnArgs);
		resourceList.emplace_back(result);
		return true;
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (" + std::to_string(spawnArgs.prefabManagerIndex) + ") prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
}

