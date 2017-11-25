#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"
#include "AnimationState.h"
#include "StateTransition.h"
#include "HashIndex.h"

//*******************************************
//			eAnimationController
// Handles sequencing of image data
// for owner->renderImage through eStateNodes
//*******************************************
class eAnimationController : public eComponent {
public:

//	friend class eAnimationControllerManager;		// sole access to AddXYZ functions
													// FIXME(!): why bother with the fns? and not just use the params themselves?

public:

														eAnimationController(eGameObject * owner);

	bool												Init(const char * filename);	// have eAnimationControllerManager::LoadController call the private AddXYZ methods		
	void												Update();
	void												Pause(bool isPaused = true);

	virtual int											GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

	void												AddAnimationState(eAnimationState && newState);
	void												AddTransition(eStateTransition && newTransition);
	void												AddIntParameter(std::string, int initialValue);
	void												AddBoolParameter(std::string, bool initialValue);
	void												AddFloatParameter(std::string, float initialValue);
	void												AddTriggerParameter(std::string);					// always constructs w/value == false
	bool												CheckTransitionConditions(const eStateTransition & transition);

private:

	std::vector<eAnimationState>						animationStates;
	eHashIndex											transitionsHash;		// allows hash collisions, indexed by eStateTransition::fromState
	std::vector<eStateTransition>						stateTransitions;

	// controller params compared against eStateTransitions and eBlendStates
	// first == user-defined parameter name, second == its value
	std::unordered_map<std::string, float>				floatParameters;
	std::unordered_map<std::string, int>				intParameters;			
	std::unordered_map<std::string, bool>				boolParameters;			// retians value until changed by user
	std::unordered_map<std::string, bool>				triggerParameters;		// resets to false after currentState updates

	int													currentState	= 0;
	bool												paused			= false;

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
// eAnimationController::Pause
// stops animation on the currentFrame
//************
inline void eAnimationController::Pause(bool isPaused) {
	paused = isPaused;
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */

