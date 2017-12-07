#include "sHero.h"
#include "Game.h"

void sHero::Think() {
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
// sHero::SpawnCopy
// copies a prefab sHero and adds unique details
//***************
bool sHero::SpawnCopy(const eVec3 & worldPosition) {
	auto & newHero = std::make_unique<sHero>(*this);
	newHero->SetZPosition(worldPosition.z);
	newHero->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	return (game.AddEntity(std::move(newHero)) >= 0);
}