#include "AnimationController.h"
#include "Game.h"

//************
// eAnimationController::eAnimationController
// copy ctor needs to allocate new state unique_ptrs
// because animationStates is a std::vector<std::unique_ptr>
//************
eAnimationController::eAnimationController(const eAnimationController & other)
	: eResource(other), eComponent(other),
	  transitionsHash(other.transitionsHash),
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
	  paused(other.paused) {
	for (auto & state : other.animationStates) {
		if (state->GetClassType() == CLASS_ANIMATIONSTATE) {
			animationStates.emplace_back(std::make_unique<eAnimationState>(*static_cast<eAnimationState *>(state.get())));
		} else if (state->GetClassType() == CLASS_BLENDSTATE){
			animationStates.emplace_back(std::make_unique<eBlendState>(*static_cast<eBlendState *>(state.get())));
		}
	}
}

/*
//************
// eAnimationController::eAnimationController
//************
eAnimationController::eAnimationController(eAnimationController && other)
	: eResource(std::move(other)), eComponent(std::move(other)),
	  transitionsHash(std::move(other.transitionsHash)),
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
	  paused(other.paused) {
}

//************
// eAnimationController::operator=
// copy-and-swap
//************
eAnimationController & eAnimationController::operator=(eAnimationController other) {
	eResource::operator=(other);
	eComponent::operator=(other);
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
}
*/

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
		if (!updateState)
			return false;
	}

	for (auto & conditionTuple : transition.intConditions) {
		updateState = eMath::CompareUtility<int>( intParameters[std::get<0>(conditionTuple)], 
												  std::get<COMPARE_ENUM>(conditionTuple), 
												  std::get<2>(conditionTuple)
												);
		if (!updateState)
			return false;
	}

	for (auto & conditionPair : transition.boolConditions) {
		updateState = (boolParameters[conditionPair.first] == conditionPair.second);
	}

	static std::vector<size_t> resetTriggers;						// static to reduce dynamic allocations
	resetTriggers.clear();										// lazy clearing
	for (size_t i = 0; i < transition.triggerConditions.size(); ++i) {
		const auto & conditionPair = transition.triggerConditions[i];
		updateState = (triggerParameters[conditionPair.first] == conditionPair.second);
		if (updateState)
			resetTriggers.emplace_back(i);
		else
			return false;
	}

	currentState = transition.toState;
	animationStates[currentState]->SetNormalizedTime(transition.offset);
	for (auto & triggerIndex : resetTriggers)					// reset only consumed triggers
		triggerParameters[triggerIndex] = false;

	return true;
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
	const int hashkey = animationStates[currentState]->nameHash;
	for (int i = transitionsHash.First(hashkey); i != -1; i = transitionsHash.Next(i)) {
		if ((stateTransitions[i].fromState == currentState) &&
			 CheckTransitionConditions(stateTransitions[i])) {
				break;
		}
	}

	 animationStates[currentState]->Update();
}

//**************
// eAnimationController::InitHashIndexes
// minimizes memory footprint, and hash collisions, and number of dynamic allocation calls
//**************
void eAnimationController::Init(int numStates, int numTransitions, int numInts, int numFloats, int numBools, int numTriggers) {
	statesHash.ClearAndResize(numStates);
	transitionsHash.ClearAndResize(numTransitions);
	intParamsHash.ClearAndResize(numInts);
	floatParamsHash.ClearAndResize(numFloats);
	boolParamsHash.ClearAndResize(numBools);
	triggerParamsHash.ClearAndResize(numTriggers);
	animationStates.reserve(numStates);
	stateTransitions.reserve(numTransitions);
	intParameters.reserve(numInts);
	floatParameters.reserve(numFloats);
	boolParameters.reserve(numBools);
	triggerParameters.reserve(numTriggers);
}

//***********************
// eAnimationController::AddAnimationState
// does not modify any states if newState::name already exists, and returns false
// otherwise adds the new state to *this and returns true
//***********************
bool eAnimationController::AddAnimationState(std::unique_ptr<eStateNode> && newState) {
	if (statesHash.First(newState->nameHash) > -1)
		return false;
	
	statesHash.Add(newState->nameHash, animationStates.size());
	animationStates.emplace_back(std::move(newState));
	return true;
}

//***********************
// eAnimationController::AddTransition
// DEBUG: always adds the transition, even if it has the same fromState, or fromState::nameHash
//***********************
void eAnimationController::AddTransition(eStateTransition && newTransition) {
	stateTransitions.emplace_back(std::move(newTransition));
}	

//***********************
// eAnimationController::SortAndHashTransitions
// transitions that occur from anyState are arranged so their conditions are checked first
//***********************
void eAnimationController::SortAndHashTransitions() {
	QuickSort( stateTransitions.data(), 
			   stateTransitions.size(),
				[](auto && a, auto && b) { 
					if (a.anyState && !b.anyState) return -1;
					else if (!a.anyState && b.anyState) return 1;
					return 0; 
	});

	for (size_t i = 0; i < stateTransitions.size(); ++i) {
		const int hashKey = animationStates[stateTransitions[i].fromState]->nameHash;
		transitionsHash.Add(hashKey, i);
	}
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
bool eAnimationController::AddTriggerParameter(const std::string & name, bool initialValue) {
	const int hashKey = triggerParamsHash.GetHashKey(name);
	if (triggerParamsHash.First(hashKey) > -1)
		return false;
	
	triggerParamsHash.Add(hashKey, triggerParameters.size());
	triggerParameters.emplace_back(initialValue);
	return true;
}

//***********************
// eAnimationController::SetOwner
// ensures that if *this was copied or moved that
// all owner and stateMachine backpointers are valid
//***********************
void eAnimationController::SetOwner(eGameObject * newOwner) {
	owner = newOwner;
	for (auto & state : animationStates)
		state->SetAnimationController(this);
}