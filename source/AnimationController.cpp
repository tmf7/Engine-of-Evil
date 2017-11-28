#include "AnimationController.h"
#include "Game.h"

//************
// eAnimationController::eAnimationController
// copy ctor needs to allocate new state unique_ptrs
// because animationStates is a std::vector<std::unique_ptr>
//************
eAnimationController::eAnimationController(const eAnimationController & other)
	: transitionsHash(other.transitionsHash),
	  statesHash(other.statesHash),
	  stateTransitions(other.stateTransitions),
	  floatParamsHash(other.floatParamsHash),
	  intParamsHash(other.intParamsHash),
	  boolParamsHash(other.boolParamsHash),
	  triggerParamsHash(other.triggerParamsHash),
	  floatParameters(other.floatParameters),
	  intParameters(other.intParameters),
	  boolParameters(other.boolParameters),
	  triggerParameters(other.triggerParameters),
	  currentState(other.currentState),
	  paused(other.paused),
	  name(other.name) {
	for (auto & state : other.animationStates) {
		if (state->GetClassType() == CLASS_ANIMATIONSTATE) {
			animationStates.emplace_back(std::make_unique<eAnimationState>(*static_cast<eAnimationState *>(state.get())));
		} else if (state->GetClassType() == CLASS_BLENDSTATE){
			animationStates.emplace_back(std::make_unique<eBlendState>(*static_cast<eBlendState *>(state.get())));
		}
	}
}

//************
// eAnimationController::eAnimationController
//************
eAnimationController::eAnimationController(eAnimationController && other)
	: transitionsHash(std::move(other.transitionsHash)),
	  statesHash(std::move(other.statesHash)),
	  animationStates(std::move(other.animationStates)),
	  stateTransitions(std::move(other.stateTransitions)),
	  floatParamsHash(std::move(other.floatParamsHash)),
	  intParamsHash(std::move(other.intParamsHash)),
	  boolParamsHash(std::move(other.boolParamsHash)),
	  triggerParamsHash(std::move(other.triggerParamsHash)),
	  floatParameters(std::move(other.floatParameters)),
	  intParameters(std::move(other.intParameters)),
	  boolParameters(std::move(other.boolParameters)),
	  triggerParameters(std::move(other.triggerParameters)),
	  currentState(other.currentState),
	  paused(other.paused),
	  name(std::move(other.name)) {
}

//************
// eAnimationController::operator=
// copy-and-swap
//************
eAnimationController & eAnimationController::operator=(eAnimationController other) {
	std::swap(transitionsHash, other.transitionsHash);
	std::swap(statesHash, other.statesHash);
	std::swap(animationStates, other.animationStates);
	std::swap(stateTransitions, other.stateTransitions);
	std::swap(floatParamsHash, other.floatParamsHash);
	std::swap(intParamsHash, other.intParamsHash);
	std::swap(boolParamsHash, other.boolParamsHash);
	std::swap(triggerParamsHash, other.triggerParamsHash);
	std::swap(floatParameters, other.floatParameters);
	std::swap(intParameters, other.intParameters);
	std::swap(boolParameters, other.boolParameters);
	std::swap(triggerParameters, other.triggerParameters);
	std::swap(currentState, other.currentState);
	std::swap(paused, other.paused);
	std::swap(name, other.name);
}

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
	if (animationStates[currentState]->GetNormalizedTime() < transition.exitTime)
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
		animationStates[currentState]->SetNormalizedTime(transition.offset);
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
		if (!transition.anyState || CheckTransitionConditions(transition))
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

	 animationStates[currentState]->Update();
}

//***********************
// eAnimationController::AddFloatParameter
// does not modify any parameters if name already exists, and returns false
// otherwise constructs the new parameter in-place and returns true
//***********************
bool eAnimationController::AddFloatParameter(const std::string & name, float initialValue) {
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
bool eAnimationController::AddIntParameter(const std::string & name, int initialValue) {
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
bool eAnimationController::AddBoolParameter(const std::string & name, bool initialValue) {
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