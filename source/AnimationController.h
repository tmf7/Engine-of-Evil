#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"
#include "AnimationState.h"

typedef struct transition_s {
	bool anyState;			// previousState always == "anystate" pseudo-AnimationState
	eAnimationState *	prevState;
	eAnimationState *	nextState;
	std::string			name;
	size_t				nameHash;

	// TODO: give this a variable list of params w/values that sync up with CONTROLLER param list, but only pay attention to certain ones
	// (ie TriggerValues to compare to Controller param values)
	// TODO: one state can have more than one or two transitions, so check them all during Update

} transition_t;



//*******************************************
//			eAnimationController
// Handles sequencing of image data
// for owner->renderImage
//*******************************************
class eAnimationController : public eComponent {
public:

														eAnimationController(eGameObject * owner);

														// TODO: either start with a vector of loaded animationstates and transitions
														// or repeatedly call AddState, AddTransition
	bool												Init(const char * filename);		
	void												Update();
	void												Pause(bool isPaused = true);

	virtual int											GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

	std::vector<std::shared_ptr<eAnimationState>>		animationStates;
//	std::vector<transition_t>							stateTransitions;		// TODO: attach transitions to eAnimationStates? (then how would controller params line up?)

	std::shared_ptr<eAnimationState>					currentState	= nullptr;
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

//************
// eAnimationController::Init
// TODO: initialize using both a behavior pattern (for animation switching via parameters)
// and a vector of eAnimation pointers
// the vector a new shared_ptr from eAnimationManager, and behavior a new unique_ptr from eAnimationContorllerManager (so different entities don't trigger at the same time)
// TODO: define ControllerBehaviors as a list of file-defined params and callback functions using those params
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

	// TODO: check all param values against all transition values ATTACHED to the CURRENT ANIMATIONSTATE ???
	// to decide if the state should change

	currentState->Update();
}

//************
// eAnimationController::Pause
// stops animation on the currentFrame
//************
inline void eAnimationController::Pause(bool isPaused) {
	paused = isPaused;
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */

