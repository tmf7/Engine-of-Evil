#include "sHero.h"
#include "Game.h"

void sHero::ExtendedThink() {

	// TODO: actually spawn this type of eEntity, not just a base eEntity

	auto & velocity = collisionModel->GetVelocity();
	eVec2 facingDirection = vec2_oneZero;
	if (velocity != vec2_zero) {
		facingDirection = velocity.Normalized();			// TODO: possibly no need to normalize this ( although controller uses [0 0.5)[0.5 1] )
		oldFacingDirection = velocity;
	} else {
		facingDirection = oldFacingDirection * 0.25f;
	}
			
	animationController->SetFloatParameter(xSpeedParameterHash, facingDirection.x);
	animationController->SetFloatParameter(ySpeedParameterHash, facingDirection.y);
	animationController->SetFloatParameter(magnitudeParameterHash, facingDirection.Length());
}