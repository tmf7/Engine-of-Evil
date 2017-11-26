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
	for (auto & conditionTuple : transition.floatConditions) {
		updateState = eMath::CompareUtility<float>( floatParameters[std::get<0>(conditionTuple)],
													std::get<COMPARE_ENUM>(conditionTuple), 
													std::get<2>(conditionTuple)
												  );
	}
	for (auto & conditionTuple : transition.intConditions) {
		updateState = eMath::CompareUtility<int>( intParameters[std::get<0>(conditionTuple)], 
												  std::get<COMPARE_ENUM>(conditionTuple), 
												  std::get<2>(conditionTuple)
												);
	}
	for (auto & conditionPair : transition.boolConditions) {
		updateState = (boolParameters[conditionPair.first] == conditionPair.second);
	}
	for (auto & conditionPair : transition.triggerConditions) {
		updateState = (triggerParameters[conditionPair.first] == conditionPair.second);
	}

	if (updateState) {
		currentState = transition.toState;
		animationStates[currentState].SetNormalizedTime(transition.offset);
		for (auto & trigger : triggerParameters)
			trigger = false;
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
	int hashkey = transitionsHash.GetHashKey(currentState);
	for (int i = transitionsHash.First(hashkey); i != -1; i = transitionsHash.Next(i)) {
		if ((stateTransitions[i].fromState == currentState) &&
			 CheckTransitionConditions(stateTransitions[i])) {
				break;
		}
	}

	// TODO: handle eBlendStates to flip their active animation and retain normalized time

	 animationStates[currentState].Update();
}

//***********************
// eAnimationController::AddFloatParameter
// does not modify any parameters if name already exists, and returns false
// otherwise constructs the new parameter in-place and returns true
//***********************
bool eAnimationController::AddFloatParameter(const std::string & name, float initialValue = 0.0f) {
	const int hashKey = floatParamsHash.GetHashKey(name);
	if (floatParamsHash.First(hashKey) > -1)
		return false;
	
	floatParamsHash.Add(hashKey, floatParameters.size());
	floatParameters.emplace_back(initialValue);
	return true;
}	 
	 
//***********************
// eAnimationController::AddIntParameter
// does not modify any parameters if name already exists, and returns false
// otherwise constructs the new parameter in-place and returns true
//*********************** 
bool eAnimationController::AddIntParameter(const std::string & name, int initialValue = 0) {
	const int hashKey = intParamsHash.GetHashKey(name);
	if (intParamsHash.First(hashKey) > -1)
		return false;
	
	intParamsHash.Add(hashKey, intParameters.size());
	intParameters.emplace_back(initialValue);
	return true;
}	 
	 
//***********************
// eAnimationController::AddBoolarameter
// does not modify any parameters if name already exists, and returns false
// otherwise constructs the new parameter in-place and returns true
//***********************
bool eAnimationController::AddBoolParameter(const std::string & name, bool initialValue = false) {
	const int hashKey = boolParamsHash.GetHashKey(name);
	if (boolParamsHash.First(hashKey) > -1)
		return false;
	
	boolParamsHash.Add(hashKey, boolParameters.size());
	boolParameters.emplace_back(initialValue);
	return true;
}

//***********************
// eAnimationController::AddTriggerParameter
// does not modify any parameters if name already exists, and returns false
// otherwise constructs the new parameter in-place and returns true
//***********************
bool eAnimationController::AddTriggerParameter(const std::string & name) {
	const int hashKey = triggerParamsHash.GetHashKey(name);
	if (triggerParamsHash.First(hashKey) > -1)
		return false;
	
	triggerParamsHash.Add(hashKey, triggerParameters.size());
	triggerParameters.emplace_back(false);
	return true;
}