#include "AnimationController.h"
#include "Game.h"

//************
// eAnimationController::Init
// TODO: this should take an index or name of a controller
// in eAnimationControllerManager that this can COPY (as in prefabs)
//************
bool eAnimationController::Init(const char * filename) {
	// TODO: implement
	return false;
}

//************
// eAnimationController::CheckTransitionConditions
// all conditions must be met for the transition to trigger
//************
bool eAnimationController::CheckTransitionConditions(const eStateTransition & transition) {
	if (animationStates[currentState].GetNormalizedTime() < transition.exitTime)
		return false;

	bool updateState = true;
	for (auto & floatCondition : transition.floatConditions) {
		auto & controllerValue = floatParameters[floatCondition.first];
		updateState = eMath::CompareUtility<float>(controllerValue, floatCondition.second.second, floatCondition.second.first);
	}
	for (auto & intCondition : transition.intConditions) {
		auto & controllerValue = intParameters[intCondition.first];
		updateState = eMath::CompareUtility<int>(controllerValue, intCondition.second.second, intCondition.second.first);

	}
	for (auto & boolCondition : transition.boolConditions) {
		auto & controllerValue = boolParameters[boolCondition.first];
		updateState = (controllerValue == boolCondition.second);
					
	}
	for (auto & triggerCondition : transition.triggerConditions) {
		auto & controllerValue = triggerParameters[triggerCondition.first];
		updateState = (controllerValue == triggerCondition.second);
	}

	if (updateState) {
		currentState = transition.toState;
		animationStates[currentState].SetNormalizedTime(transition.offset);
		for (auto & trigger : triggerParameters)
			trigger.second = false;
	}
	return updateState;
}

//************
// eAnimationController::Update
// updates the current state of animation
// must be unpaused to fully animate
//************
void eAnimationController::Update() {
	if (paused)
		return;

	for (auto & transition : stateTransitions) {
		if (!transition.anyState)
			break;

		if (CheckTransitionConditions(transition))
			break;
	}

	// DEBUG: currentState transition checks are still allowed even if an anyState transition has triggered
	auto hasher = std::hash<int>{};
	int hashkey = hasher(currentState);
	for (int i = transitionsHash.First(hashkey); i != -1; i = transitionsHash.Next(i)) {
		if ((stateTransitions[i].fromState == currentState) &&
			 CheckTransitionConditions(stateTransitions[i])) {
				break;
		}
	}

	// TODO: handle eBlendStates to flip their active animation and retain normalized time

	 animationStates[currentState].Update();
}