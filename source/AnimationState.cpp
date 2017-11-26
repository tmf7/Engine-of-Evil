#include "AnimationState.h"
#include "Game.h"

//*********************
// eAnimationState::Update
//*********************
void eAnimationState::Update() {
	NextFrame(*animation);
}