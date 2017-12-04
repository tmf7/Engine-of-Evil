#include "sHero.h"
#include "Game.h"

void sHero::Think() {

	// TODO: actually spawn this type of eEntity, not just a base eEntity
	// SOLUTION: register and find a SpawnFunction by name (register on user-end via RegisterSpawnFunc(stringName, functionPointer),
	// then call in eEntityPrefabManager::LoadAndGet and/or eMap::Load)
	// SOLUTION: create a single factory method that takes a string

	auto & velocity = collisionModel->GetVelocity();
	eVec2 facingDirection;
	if (velocity != vec2_zero) {
		facingDirection = -velocity.Normalized();
		eMath::CartesianToIsometric(facingDirection.x, facingDirection.y);
		facingDirection.Normalize();
		oldFacingDirection = facingDirection;
	} else {
		facingDirection = oldFacingDirection * 0.25f;
	}
			
	animationController->SetFloatParameter(xSpeedParameterHash, facingDirection.x);
	animationController->SetFloatParameter(ySpeedParameterHash, facingDirection.y);
	animationController->SetFloatParameter(magnitudeParameterHash, facingDirection.Length());
}

//***************
// sHero::Spawn
// copies a prefab eEntity and adds unique details
//***************
bool sHero::Spawn(const eVec3 & worldPosition) {
	int	spawnID = game.AddEntity(std::make_unique<sHero>(*this));
	if (spawnID < 0)
		return false;
	
	auto & newEntity = game.GetEntity(spawnID);											
	if (&newEntity->RenderImage() != nullptr) {
		newEntity->SetStatic(false);
		newEntity->RenderImage().SetIsSelectable(true);
	}

	newEntity->SetWorldLayer(worldPosition.z);
	newEntity->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	return true;
}