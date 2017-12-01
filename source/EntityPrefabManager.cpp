#include "EntityPrefabManager.h"

//**************************
// eEntityPrefabManager::Init
// TODO: output to an error log file upon failure
//**************************
bool eEntityPrefabManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_IMAGES);

	// register the error_prefab_entity as the first element of resourceList
	entitySpawnArgs_t defaultSpawnArgs;
	int hashKey = resourceHash.GetHashKey(std::string("error_prefab_entity"));
	resourceHash.Add(hashKey, resourceList.size());
	try {
		resourceList.emplace_back(std::make_shared<eEntity>(defaultSpawnArgs));	// default error prefab
	} catch (const badEntityCtorException & e) {
		std::string message = e.what + " caused eEntity (0) prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		return false;
	}
	return true;
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
// spriteFilename=filename.eimg\n		(leave spriteFilename, renderBlockSize, and imageColliderOffset empty if entity has no visuals)
// initialSpriteFrame= int\n			(which subframe of spriteFilename to start on, defaults to 0 if out-of-bounds, gets ignored if spriteFilename is blank)
// animationController=filename.ectrl	(spriteFilename and renderBlockSize must be set if animationController is set)
// renderBlockSize= x y z\n				(floats)
// imageCollisionOffset= x y\n			(floats)
// localBounds= xMin yMin xMax yMax\n	(floats, mins = -maxs avoids allocating an eCollisionModel on the eEntity, collisionActive and movementSpeed will be ignored.)
// movementSpeed= scalarValue\n			(float, set to 0 to avoid allocating an eMovementPlanner on the eEntity)
// collisionActive= trueOrFalseHere\n	(boolalpha)
// [NOTE]: batch entity prefab files are .bprf
//**************************
bool eEntityPrefabManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eEntity> & result) {
	// prefab already loaded
	if ((result = Get(resourceFilename))->IsValid())
		return true;

	std::ifstream	read(resourceFilename);
	// unable to find/open file
	if (!read.good()) {
		result = resourceList[0];			// default error prefab
		return false;
	}

	entitySpawnArgs_t spawnArgs;
	spawnArgs.sourceFilename = resourceFilename;

	char buffer[MAX_ESTRING_LENGTH];
	read.ignore(std::numeric_limits<std::streamsize>::max(), '=');		// spriteFilename	(skips all line comments '#')
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if (!VerifyRead(read)) {
		result = resourceList[0];
		return false;
	}

	spawnArgs.spriteFilename = buffer;
	if (!spawnArgs.spriteFilename.empty()) {

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// initialSpriteFrame
		read >> spawnArgs.initialSpriteFrame;
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// animationController
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '\n');
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		spawnArgs.animationControllerFilename = buffer;

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// renderBlockSize
		read >> spawnArgs.renderBlockSize.x;
		read >> spawnArgs.renderBlockSize.y;
		read >> spawnArgs.renderBlockSize.z;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// imageCollisionOffset
		read >> spawnArgs.imageColliderOffset.x;
		read >> spawnArgs.imageColliderOffset.y;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read)){
			result = resourceList[0];
			return false;
		}

	} else {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	read.ignore(std::numeric_limits<std::streamsize>::max(), '=');		// localBounds
	read >> spawnArgs.localBounds[0][0];
	read >> spawnArgs.localBounds[0][1];
	read >> spawnArgs.localBounds[1][0];
	read >> spawnArgs.localBounds[1][1];
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	if (!VerifyRead(read)) {
		result = resourceList[0];
		return false;
	}

	if (!spawnArgs.localBounds.IsEmpty()) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// movementSpeed
		read >> spawnArgs.movementSpeed;
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read)){
			result = resourceList[0];
			return false;
		}

		read.ignore(std::numeric_limits<std::streamsize>::max(), '=');	// collisionActive
		read >> std::boolalpha >> spawnArgs.collisionActive;
//		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read)){
			result = resourceList[0];
			return false;
		}
	}
	read.close();
	spawnArgs.prefabManagerIndex = resourceList.size();

	// register the requested entity prefab
	try {
		result = std::make_shared<eEntity>(spawnArgs);
		resourceHash.Add(result->GetNameHash(), resourceList.size());
		resourceList.emplace_back(result);
		return true;
	} catch (const badEntityCtorException & e) {
		// TODO: output to an error log file (popup is fine for now because it's more obvious and immediate)
		std::string message = e.what + " caused eEntity (" + std::to_string(spawnArgs.prefabManagerIndex) + ") prefab failure.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message.c_str(), NULL);
		result = resourceList[0];		// default error prefab
		return false;
	}
}

