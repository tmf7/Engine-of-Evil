#include <functional>		// std::hash
//#include "EntityPrefabManager.h"
#include "Game.h"

//***************
// eEntityPrefabManager::Init
//***************
bool eEntityPrefabManager::Init() {
	// prepare the hashindex
	prefabFilenameHash.ClearAndResize(MAX_IMAGES);

	// register the error_prefab_entity as the first element of prefabList
	entitySpawnArgs_t defaultSpawnArgs;
	auto hasher = std::hash<std::string>{};
	prefabFilenameHash.Add(hasher("error_prefab_entity"), prefabList.size());
	try {
		prefabList.push_back(std::make_shared<eEntity>(defaultSpawnArgs));	// error prefab
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (0) prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
	return true;
}

//***************
// eEntityPrefabManager::BatchLoadPrefabs
// loads a batch of eneity prefab resources
// user can optionally call entityPrefabManager.Clear()
// prior to this to facilitate starting with a fresh set of prefabs
// TODO: allow selective unloading of prefabs (eg: std::shared_ptr already does reference counting
// take those numbers and add/subtract according to the next level's filename batch)
// DEBUG (.bepf file format):
// 1=prefabFilename\n	(prefabList index used when spawning eEntities) (everything past the first string is ignored)
// 2=prefabFilename\n	(the 0th index is the error eEntity)
// (repeat)
//***************
bool eEntityPrefabManager::BatchLoadPrefabs(const char * prefabBatchFile) {
	std::shared_ptr<eEntity> tempResult;	// DEBUG: not acually used, but necessary for LoadImage
	char filename[MAX_ESTRING_LENGTH];
	std::ifstream	read(prefabBatchFile);

	// unable to find/open file
	if(!read.good())
		return false;

	while (!read.eof()) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// skip prefabList index comment
		read >> filename;
		if (!VerifyRead(read))
			return false;

		if (!LoadPrefab(filename, tempResult))
			return false;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//***************
// eEntityPrefabManager::GetPrefab
// fill the result with an eEntity pointer if it exists
// if filename is null or the eEntity doesn't exist
// then it result fills with an error eEntity pointer
//***************
bool eEntityPrefabManager::GetPrefab(const char * filename, std::shared_ptr<eEntity> & result) {
	if (!filename) {
		result = prefabList[0]; // error prefab
		return false;
	}

	// search for pre-existing texture
	auto hasher = std::hash<std::string>{};
	int hashkey = hasher(filename);
	for (int i = prefabFilenameHash.First(hashkey); i != -1; i = prefabFilenameHash.Next(i)) {
		if (prefabList[i]->GetPrefabFilename() == filename) {
			result = prefabList[i];
			return true;
		}
	}
	result = prefabList[0]; // error prefab
	return false;
}

//***************
// eEntityPrefabManager::GetPrefab
// fill the result with an eEntity pointer if it exists
// if guid is negative or beyond the number of loaded images
// then it result fills with an error eEntity pointer
//***************
bool eEntityPrefabManager::GetPrefab(int prefabID, std::shared_ptr<eEntity> & result) {
	if (prefabID < 0 && prefabID > prefabList.size()) {		// DEBUG: prefabID will never be larger than max signed int
		result = prefabList[0]; // error prefab
		return false;
	}
	result = prefabList[prefabID];
	return true;
}


//***************
// eEntityPrefabManager::LoadPrefab
// attempts to load the given entity prefab file
// sets result to found prefab on success
// sets result to error prefab
// and returns false on failure
// DEBUG (.eprf file format):
// # first line comment\n 
// spriteFilename=filename.png\n		(leave spriteFilename, renderBlockSize, and imageColliderOffset empty if entity has no visuals)
// renderBlockSize= x y z\n				(floats)
// imageCollisionOffset= x y\n			(floats)
// localBounds= xMin yMin xMax yMax\n	(floats, mins = -maxs avoids allocating an eCollisionModel on the eEntity, collisionActive and movementSpeed will be ignored.)
// movementSpeed= scalarValue\n			(float, set to 0 to avoid allocating an eMovement on the eEntity)
// collisionActive= trueOrFalseHere\n	(boolalpha)
//***************
bool eEntityPrefabManager::LoadPrefab(const char * filename, std::shared_ptr<eEntity> & result) {
	// check if the prefab already exists 
	// and set result to that if it does
	if (GetPrefab(filename, result))
		return true;

	std::ifstream	read(filename);
	// unable to find/open file
	if (!read.good()) 
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip first line comment

	entitySpawnArgs_t spawnArgs;
	spawnArgs.prefabFilename = filename;
	char buffer[MAX_ESTRING_LENGTH];

	read.ignore(std::numeric_limits<std::streamsize>::max(), '=');		// spriteFilename
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read))
		return false;

	spawnArgs.spriteFilename = buffer;
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
	spawnArgs.prefabManagerIndex = prefabList.size();

	// register the requested entity prefab
	auto hasher = std::hash<std::string>{};
	prefabFilenameHash.Add(hasher(filename), prefabList.size());
	try {
		result = std::make_shared<eEntity>(spawnArgs);
		prefabList.push_back(result);
		return true;
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (" + std::to_string(spawnArgs.prefabManagerIndex) + ") prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
}

//***************
// eEntityPrefabManager::Clear
// clears all pointers to the current set 
// of resource entity prefabs, which allows them
// to be deleted once no object is using them,
// allows for new resource prefabs to load
// without using excessive memory
//***************
void eEntityPrefabManager::Clear() {
	prefabList.clear();
	prefabFilenameHash.Clear();
}