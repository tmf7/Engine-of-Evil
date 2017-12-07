#include "CreatePrefabStrategies.h"
#include "sHero.h"

//***************
// eCreateEntityPrefabUser::CreatePrefab
//***************
bool eCreateEntityPrefabUser::CreatePrefab(std::shared_ptr<eEntity> & newPrefab, const std::string & prefabShortName, const eDictionary & spawnArgs) {
	if (prefabShortName == "Entity")
		newPrefab = std::make_shared<eEntity>();
	else if (prefabShortName == "sHero")
		newPrefab = std::make_shared<sHero>();
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