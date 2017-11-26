#ifndef EVIL_STATE_TRANSITION_H
#define EVIL_STATE_TRANSITION_H

#include "Definitions.h"

/*

	// TODO: make a base/template eResourceManager class because so all 3 managers use the same functionality, just different data types

	// TODO: std::sort the stateTransitions vector according to anyState bool AFTER all transitions have been loaded, then update transitionHash (in eAnimController)

	// TODO: transitions can EITHER occur at any point, or wait until the state normalizedTime == ExitTime [==1.0f usually]),
	// and only check the transition parameter values AFTER normalizedTime >= ExitTime (if any)

	// TODO: eTransitions always start the new state from the beginning of its animation (regardless of "Exit Time") (unless I also add a transition offset for where to start the new normalizedTime),
	// while eBlendStates start the selected animation at the same normalized time, right away

	// TODO: create a base eStateNode class, and derive eAnimationState and eBlendState
	// eAnimationState will have only one eAnimation to Update
	// eBlendState will have a vector of eAnimations to select from based on 1 or 2 (not more) float_type parameters (only floats, not ints or bools)
	// from its eAnimationController owner (as float *) (NOTE: at least one float must be selected, if not in load-file, then default to first floatParameter)
	// AND the selection will be based on lowest squaredDistance from controllerParam(s) position to blendNode param(s) position
	// AND the blendNode params will be 1 or 2 floats PER eAnimation in the eBlendState (directly proportional to controllerParams being tracked)
	// and associated fixed-values (for getting the distance to)
*/

//**************************************
//		eStateTransition
// used by eAnimationController to control
// the flow between its various eStateNodes
//***************************************
class eStateTransition {
public:

	// the only classes with access to transition values
	friend class eAnimationControllerManager;
	friend class eAnimationController;

public:

							eStateTransition(const std::string & name, 
											 bool anyState, int fromStateIndex, 
											 int toStateIndex,
											 float exitTime = 0.0f, 
											 float offset = 0.0f);

private:

	void					AddFloatCondition(int controllerFloatIndex, COMPARE_ENUM compare, float value);
	void					AddIntCondition(int controllerIntIndex, COMPARE_ENUM compare, int value);
	void					AddBoolCondition(int controllerBoolIndex, bool value);
	void					AddTriggerCondition(int controllerTriggerIndex);

private:

	std::string				name;
	size_t					nameHash;
	float					exitTime;			// currentState::normalizedTime to start checking conditions (if <= 0.0f, there MUST be at least one transition param)
	float					offset;				// the normalizedTime to start playing at in toState
	bool					anyState;			// all conditions checked regardless of eAnimationController::currentState (DEBUG: ie: ignores fromState)
	int						fromState;			// index within eAnimationController::animationStates this is attached to  (DEBUG: ignored if anyState == true)
	int						toState;			//   "     "           "                   "          this modifies eAnimationController::currentState to

	std::vector<std::tuple<int, COMPARE_ENUM, float>>	floatConditions;
	std::vector<std::tuple<int, COMPARE_ENUM, int>>		intConditions;		
	std::vector<std::pair<int, bool>>					boolConditions;		// values can be true or false
	std::vector<std::pair<int, bool>>					triggerConditions;  // all values are true
};

//*******************
// eStateTransition::eStateTransition
//*******************
inline eStateTransition::eStateTransition(const std::string & name, bool anyState, int fromStateIndex, int toStateIndex, float exitTime, float offset) 
	: name(name),
	  anyState(anyState),
	  fromState(fromStateIndex),
	  toState(toStateIndex),
	  exitTime(exitTime),
	  offset(offset) {
	nameHash = std::hash<std::string>()(name);
}

//*******************
// eStateTransition::AddFloatCondition
// takes the index within eAnimationController::floatParameters to compare
// to param value according to param compare
//*******************
inline void eStateTransition::AddFloatCondition(int controllerFloatIndex, COMPARE_ENUM compare, float value) {
	floatConditions.emplace_back(std::make_tuple(controllerFloatIndex, compare, value));
}

//*******************
// eStateTransition::AddIntCondition
// takes the index within eAnimationController::intParameters to compare
// to param value according to param compare
//*******************
inline void eStateTransition::AddIntCondition(int controllerIntIndex, COMPARE_ENUM compare, int value) {
	intConditions.emplace_back(std::make_tuple(controllerIntIndex, compare, value));
}

//*******************
// eStateTransition::AddBoolCondition
// takes the index within eAnimationController::boolParameters to compare
// to param value according to a "==" operation
//*******************
inline void eStateTransition::AddBoolCondition(int controllerBoolIndex, bool value) {
	boolConditions.emplace_back(std::make_pair(controllerBoolIndex, value));
}

//*******************
// eStateTransition::AddTriggerCondition
// takes the index within eAnimationController::triggerParameters to compare
// to "true" according to a "==" operation
//*******************
inline void eStateTransition::AddTriggerCondition(int controllerTriggerIndex) {
	triggerConditions.emplace_back(std::make_pair(controllerTriggerIndex, true));
}

#endif  /* EVIL_STATE_TRANSITION_H */