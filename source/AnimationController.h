#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"
#include "AnimationState.h"

//*******************************************
//			eAnimationController
// Handles sequencing of image data
// for owner->renderImage
//*******************************************
class eAnimationController : public eComponent {
public:

									eAnimationController(eGameObject * owner);

	bool							Init(const char * filename);
	void							Update();
	void							Pause(bool isPaused = true);

	virtual int						GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

	typedef std::shared_ptr<eAnimationState>	AnimSSPtr_t;
	std::vector<AnimSSPtr_t>		animationStates;
	
	// TODO: this should track a normalized time (or just time) proportional to gameTime (or frameTime) to check against currentAnimationFrame.normalizedTime
	// to see if it should be pushed into owner->renderImage::image and owner->renderImage::srcRect

	int								currentFrame	= 0;
	int								delayCounter	= 0;
	bool							paused			= true;

	// experimental
	std::string						name;					// unique name relative to owner "melee_32_controller"
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

	delayCounter++;
	if (delayCounter > frameDelay) {
		delayCounter = 0;
		currentFrame++;
	}

	if (currentFrame > lastFrame)
		currentFrame = firstFrame;
}

//************
// eAnimationController::Pause
// stops animation on the currentFrame
//************
inline void eAnimationController::Pause(bool isPaused) {
	paused = isPaused;
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */

