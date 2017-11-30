#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"
#include "Resource.h"
#include "AnimationState.h"
#include "BlendState.h"
#include "StateTransition.h"
#include "HashIndex.h"


//*******************************************
//			eAnimationController
// Handles sequencing of image data
// for owner->renderImage through eStateNodes
// FIXME: call eComponent::SetOwner call upon COPYING (ie std::make_unique) a prefab into an eGameObject
//*******************************************
class eAnimationController : public eComponent, public eResource {
public:

	friend class eAnimationControllerManager;		// sole access to Add/GetXYZParameterIndex functionality

public:

	virtual							   ~eAnimationController() = default;
										eAnimationController() = default;
										eAnimationController(const eAnimationController & other);
										eAnimationController(eAnimationController && other);
										eAnimationController(const std::string & sourceFilename, int managerIndex);

	eAnimationController &				operator=(eAnimationController other);


	void								Update();
	void								Pause(bool isPaused = true);

	// returns true if the item exists and can be set, or false if it doesn't exist
	const eStateNode &					GetCurrentState() const;
	eStateNode &						GetCurrentState();

	bool								SetFloatParameter(const std::string & name, float newValue);
	bool								SetIntParameter(const std::string & name, int newValue);
	bool								SetBoolParameter(const std::string & name, bool newValue);
	bool								SetTriggerParameter(const std::string & name);
	bool								ResetTriggerParameter(const std::string & name);
	bool								SetFloatParameter(int nameHash, float newValue);
	bool								SetIntParameter(int nameHash, int newValue);
	bool								SetBoolParameter(int nameHash, bool newValue);
	bool								SetTriggerParameter(int nameHash);
	bool								ResetTriggerParameter(int nameHash);

	// returns the value, or a default if it doesn't exist
	float								GetFloatParameter(const std::string & name) const;
	int									GetIntParameter(const std::string & name) const;
	bool								GetBoolParameter(const std::string & name) const;
	bool								GetTriggerParameter(const std::string & name) const;
	float								GetFloatParameter(int nameHash) const;
	int									GetIntParameter(int nameHash) const;
	bool								GetBoolParameter(int nameHash) const;
	bool								GetTriggerParameter(int nameHash) const;

	virtual int							GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

	// returns true if add was successful, false if the item already exists 
	// eStateTransitions however allows hash collisions, because it's indexed by eStateTransition::fromState
	bool								AddAnimationState(std::unique_ptr<eStateNode> && newState);
	void								AddTransition(eStateTransition && newTransition);

	bool								AddFloatParameter(const std::string & name, float initialValue = 0.0f);
	bool								AddIntParameter(const std::string & name, int initialValue = 0);
	bool								AddBoolParameter(const std::string & name, bool initialValue = false);
	bool								AddTriggerParameter(const std::string & name, bool initialValue = false);
	bool								CheckTransitionConditions(const eStateTransition & transition);

	// TODO: these fns used to initialize eStateTransitions a load-time, for quick TransitionCondition checks during Update
	// FIXME/BUG: do not call these until ALL parameters have been initilized, otherwise they may move as std::vector resizes
	// or use std::vector::reserve(numFloatParams) etc in eAnimationControllerManager::LoadController
	// TODO: std::sort the stateTransitions vector according to anyState bool AFTER all transitions have been loaded, then update transitionHash (in eAnimController)
	// TODO: make sure to eHashIndex::ClearAndResize each eAnimationController::XYZParamsHash based on the number of params at load-time to minimize memory footprint
	int									GetFloatParameterIndex(const std::string & name) const;
	int									GetIntParameterIndex(const std::string & name) const;
	int									GetBoolParameterIndex(const std::string & name) const;
	int									GetTriggerParameterIndex(const std::string & name) const;
	int									GetStateIndex(const std::string & name) const;

private:

	// eHashIndex allows hash collisions as needed and allows for contiguous memory footprint
	eHashIndex									transitionsHash;	// indexed by eStateTransition::fromState
	eHashIndex									statesHash;			// indexed by eAnimationState::name
	std::vector<std::unique_ptr<eStateNode>>	animationStates;
	std::vector<eStateTransition>				stateTransitions;

	// indexed by user-defined parameter name
	eHashIndex							floatParamsHash;
	eHashIndex							intParamsHash;
	eHashIndex							boolParamsHash;
	eHashIndex							triggerParamsHash;

	// controller params compared against eStateTransitions and eBlendStates
	std::vector<float>					floatParameters;
	std::vector<int>					intParameters;			
	std::vector<bool>					boolParameters;			// retains value until changed by user
	std::vector<bool>					triggerParameters;		// resets to false after currentState changes

	int									currentState	= 0;
	bool								paused			= false;

};

//**************
// eAnimationController::eAnimationController
//**************
inline eAnimationController::eAnimationController(const std::string & sourceFilename, int animationControllerManagerIndex)
	: eResource(sourceFilename, animationControllerManagerIndex) {
}

//***********************
// eAnimationController::Pause
// stops animation on the currentFrame
//***********************
inline void eAnimationController::Pause(bool isPaused) {
	paused = isPaused;
}

//***********************
// eAnimationController::AddAnimationState
//***********************
inline bool eAnimationController::AddAnimationState(std::unique_ptr<eStateNode> && newState) {
	statesHash.Add(newState->NameHash(), animationStates.size());
	animationStates.emplace_back(std::move(newState));
}

//***********************
// eAnimationController::AddTransition
//***********************
inline void eAnimationController::AddTransition(eStateTransition && newTransition) {
	transitionsHash.Add(newTransition.fromState, stateTransitions.size());
	stateTransitions.emplace_back(std::move(newTransition));
}	

//***********************
// eAnimationController::GetCurrentState
// returns the currently active eAnimationState of this eAnimationController
//***********************
inline const eStateNode & eAnimationController::GetCurrentState() const {
	return *animationStates[currentState];
}

//***********************
// eAnimationController::GetCurrentState
// returns the currently active eAnimationState of this eAnimationController
//***********************
inline eStateNode & eAnimationController::GetCurrentState() {
	return *animationStates[currentState];
}

//***********************
// eAnimationController::SetFloatParameter
// return true if the parameter exists and can be set
// returns false if it doesn't exist
//***********************
inline bool eAnimationController::SetFloatParameter(const std::string & name, float newValue) {
	return SetFloatParameter(floatParamsHash.GetHashKey(name), newValue);
}

//***********************
// eAnimationController::SetIntParameter
// return true if the parameter exists and can be set
// returns false if it doesn't exist
//***********************
inline bool eAnimationController::SetIntParameter(const std::string & name, int newValue) {
	return SetIntParameter(intParamsHash.GetHashKey(name), newValue);
}

//***********************
// eAnimationController::SetBoolParameter
// return true if the parameter exists and can be set
// returns false if it doesn't exist
//***********************
inline bool eAnimationController::SetBoolParameter(const std::string & name, bool newValue) {
	return SetBoolParameter(boolParamsHash.GetHashKey(name), newValue);
}

//***********************
// eAnimationController::SetTriggerParameter
// return true if the parameter exists and can be set
// returns false if it doesn't exist
// setting a trigger always sets its value to true
// and maintains that value until either
// its transition completes, 
// or a user calls ResetTriggerParameter
//***********************
inline bool eAnimationController::SetTriggerParameter(const std::string & name) {
	return SetTriggerParameter(triggerParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::ResetTriggerParameter
// similar to SetTriggerParameter(std::string)
// except sets its value to false
// useful in the event a trigger is set but the transition doesn't occur
//***********************
inline bool eAnimationController::ResetTriggerParameter(const std::string & name) {
	return ResetTriggerParameter(triggerParamsHash.GetHashKey(name));
}


//***********************
// eAnimationController::SetFloatParameter
// same as SetFloatParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::SetFloatParameter(int nameHash, float newValue) {
	const int index = floatParamsHash.First(nameHash);
	if (index == -1)
		return false;
	
	floatParameters[index] = newValue;
	return true;
}

//***********************
// eAnimationController::SetIntParameter
// same as SetIntParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::SetIntParameter(int nameHash, int newValue) {
	const int index = intParamsHash.First(nameHash);
	if (index == -1)
		return false;
	
	intParameters[index] = newValue;
	return true;
}

//***********************
// eAnimationController::SetBoolParameter
// same as SetBoolParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::SetBoolParameter(int nameHash, bool newValue) {
	const int index = boolParamsHash.First(nameHash);
	if (index == -1)
		return false;
	
	boolParameters[index] = newValue;
	return true;
}

//***********************
// eAnimationController::SetTriggerParameter
// same as SetTriggerParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::SetTriggerParameter(int nameHash) {
	const int index = triggerParamsHash.First(nameHash);
	if (index == -1)
		return false;
	
	triggerParameters[index] = true;
	return true;
}

//***********************
// eAnimationController::ResetTriggerParameter
// similar to SetTriggerParameter(int)
// except sets its value to false
// useful in the event a trigger is set but the transition doesn't occur
// assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::ResetTriggerParameter(int nameHash) {
	const int index = triggerParamsHash.First(nameHash);
	if (index == -1)
		return false;
	
	triggerParameters[index] = false;
	return true;
}

//***********************
// eAnimationController::GetFloatParameter
// returns the value of the paramater with the param name if it exists
// DEBUG: returns default 0.0f if it doesn't exist
//***********************
inline float eAnimationController::GetFloatParameter(const std::string & name) const {
	return GetFloatParameter(floatParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetIntParameter
// returns the value of the paramater with the param name if it exists
// DEBUG: returns default 0 if it doesn't exist
//***********************
inline int eAnimationController::GetIntParameter(const std::string & name) const {
	return GetIntParameter(intParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetBoolParameter
// returns the value of the paramater with the param name if it exists
// DEBUG: returns default false if it doesn't exist
//***********************
inline bool eAnimationController::GetBoolParameter(const std::string & name) const {
	return GetBoolParameter(boolParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetTriggerParameter
// returns the value of the paramater with the param name if it exists
// DEBUG: returns default false if it doesn't exist
//***********************
inline bool eAnimationController::GetTriggerParameter(const std::string & name) const {
	return GetTriggerParameter(triggerParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetFloatParameter
// same as GetFloatParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline float eAnimationController::GetFloatParameter(int nameHash) const {
	const int index = floatParamsHash.First(nameHash);
	if (index == -1)
		return 0.0f;
	
	return floatParameters[index];
}

//***********************
// eAnimationController::GetIntParameter
// same as GetIntParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline int eAnimationController::GetIntParameter(int nameHash) const {
	const int index = intParamsHash.First(nameHash);
	if (index == -1)
		return 0;
	
	return intParameters[index];
}

//***********************
// eAnimationController::GetBoolParameter
// same as GetBoolParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::GetBoolParameter(int nameHash) const {
	const int index = boolParamsHash.First(nameHash);
	if (index == -1)
		return 0;
	
	return boolParameters[index];
}

//***********************
// eAnimationController::GetTriggerParameter
// same as GetTriggerParameter(std::string)
// except assumes the user has chached the hashKey
//***********************
inline bool eAnimationController::GetTriggerParameter(int nameHash) const {
	const int index = triggerParamsHash.First(nameHash);
	if (index == -1)
		return 0;
	
	return triggerParameters[index];
}

//***********************
// eAnimationController::GetFloatParameterIndex
// returns the index within eAnimationController::floatParameters
// of the named parameter if it exists
// returns -1 if it doesn't exist
// used by eAnimationControllerManager to initialize eStateTransitions
//***********************
inline int eAnimationController::GetFloatParameterIndex(const std::string & name) const {
	return floatParamsHash.First(floatParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetIntParameterIndex
// returns the index within eAnimationController::intParameters
// of the named parameter if it exists
// returns -1 if it doesn't exist
// used by eAnimationControllerManager to initialize eStateTransitions
//***********************
inline int eAnimationController::GetIntParameterIndex(const std::string & name) const {
	return intParamsHash.First(intParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetBoolParameterIndex
// returns the index within eAnimationController::boolParameters
// of the named parameter if it exists
// returns -1 if it doesn't exist
// used by eAnimationControllerManager to initialize eStateTransitions
//***********************
inline int eAnimationController::GetBoolParameterIndex(const std::string & name) const {
	return boolParamsHash.First(boolParamsHash.GetHashKey(name));

}

//***********************
// eAnimationController::GetTriggerParameterIndex
// returns the index within eAnimationController::triggerParameters
// of the named parameter if it exists
// returns -1 if it doesn't exist
// used by eAnimationControllerManager to initialize eStateTransitions
//***********************
inline int eAnimationController::GetTriggerParameterIndex(const std::string & name) const {
	return triggerParamsHash.First(triggerParamsHash.GetHashKey(name));
}

//***********************
// eAnimationController::GetStateIndex
// returns the index within eAnimationController::animationStates
// of the named state if it exists
// returns -1 if it doesn't exist
// used by eAnimationControllerManager to initialize eStateTransitions
//***********************
inline int eAnimationController::GetStateIndex(const std::string & name) const {
	return statesHash.First(statesHash.GetHashKey(name));
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */