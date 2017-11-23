#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"
#include "AnimationState.h"

typedef struct transition_s {
	bool				anyState;			// fromState always == "anystate" pseudo-AnimationState
	int					fromState;			// index within eAnimationController::animationStates this is attached to (for indexing)
	int					toState;			//   ""    ""          ""                  ""         this modifies eAnimationController::currentState to
	std::string			name;
	size_t				nameHash;

/*

	// TODO:sync these up with CONTROLLER param list, but only pay attention to certain ones according to user-definition of the transitions
	// TODO: how should the watch values compare to the param values? >, >=, ==, <=, <, !=, and how should that imported as designed by the user?
	std::unordered_map<std::string, int>	watchInts;		
	std::unordered_map<std::string, bool>	watchBools;		
	std::unordered_map<std::string, bool>	watchTriggers;	
	std::unordered_map<std::string, float>	watchFloats;	

	// TODO(!): if transitioning mid-animation (eg: during a run in a direction to change direction) then it would be a good idea
	// to start the new state at the same normalized time as the previous state
	// TODO: ALSO, should transitions be allowed to occur at any point, or wait until the state normalizedTime == ExitTime [==1.0f usually]),
	// and only check the transition parameter values AFTER normalizedTime >= ExitTime

	// TODO: lambda expression closure objects to express how int/bool/floats are user-selected to compare? instead of functors?
	
	// TODO: define a watchCondition as <name, value, userSelectedPredicate>
	// where the userSelectedPredicate is 
	std::less_equal<int> lei;
	std::less<int> li;
	std::greater<int> gi;
	std::greater_equal<int> gei;
	std::equal_to<int> ei;
	std::not_equal_to<int> nei;

	int paramtest;
	if ( std::type_index(typeid(paramtest)).hash_code() == std::type_index(typeid(int)).hash_code()) {
	} else if (std::type_index(typeid(paramtest)).hash_code() == std::type_index(typeid(bool)).hash_code()) {
	} else if (std::type_index(typeid(paramtest)).hash_code() == std::type_index(typeid(float)).hash_code()) {
	}

*/

} transition_t;



//*******************************************
//			eAnimationController
// Handles sequencing of image data
// for owner->renderImage
//*******************************************
class eAnimationController : public eComponent {
public:

														eAnimationController(eGameObject * owner);

	bool												Init(const char * filename);		
	void												Update();
	void												Pause(bool isPaused = true);

	virtual int											GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

	std::vector<std::shared_ptr<eAnimationState>>		animationStates;
/*

	std::unordered_map<int, transition_t>	stateTransitions;		// first == animationStateIndex
	std::unordered_map<std::string, int>	intParameters;			// first == user-defined parameter name, second == its value
	std::unordered_map<std::string, bool>	boolParameters;			// first == user-defined parameter name, second == its value (stays until set otherwise)
	std::unordered_map<std::string, bool>	triggerParameters;		// first == user-defined parameter name, second == its value (resets to false after currentState updates)
	std::unordered_map<std::string, float>	floatParameters;		// first == user-defined parameter name, second == its value

*/

	int													currentState	= 0;	// FIXME: "start" node of state machine shouldn't necessarily play an animation until transtion to a proper state
	bool												paused			= true;

	// experimental
	std::string											name;					// unique name relative to owner "melee_32_controller"
};

//************
// eAnimationController::eAnimationController
//************
inline eAnimationController::eAnimationController(eGameObject * owner) {
	this->owner = owner;
}

#include <typeindex>

//************
// eAnimationController::Init
// TODO: either start with a vector of loaded animationstates and transitions
// or repeatedly call AddState, AddTransition
//************
inline bool eAnimationController::Init(const char * filename) {
	// TODO: implement
	return false;
}

//************
// eAnimationController::NextFrame
// continues the current state of animation
// must be unpaused to fully animate
//************
inline void eAnimationController::Update() {
	if (paused)
		return;

	// TODO: check all param values against all transition values according to the currentState to decide if the state should change
	// TODO: reset all triggerParameters if the currentState value has changed

	animationStates[currentState]->Update();
}

//************
// eAnimationController::Pause
// stops animation on the currentFrame
//************
inline void eAnimationController::Pause(bool isPaused) {
	paused = isPaused;
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */

