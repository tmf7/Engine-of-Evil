#ifndef EVIL_ANIMATION_CONTROLLER_MANAGER_H
#define EVIL_ANIMATION_CONTROLLER_MANAGER_H

#include "AnimationController.h"
#include "ResourceManager.h"

//***********************************************
//		eAnimationControllerManager
// Handles all eAnimationController allocation and freeing
// see also: eResourceManager template
//***********************************************
class eAnimationControllerManager : public eResourceManager<eAnimationController> {
public:

	virtual bool							Init() override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eAnimationController> & result) override;

	virtual int								GetClassType() const override { return CLASS_ANIMATIONCONTROLLER_MANAGER; }
};

/*
// [[IMPORTANT]] resourceFilename will be the .eimg file path (with contained .png path and subframe data)
// FIXME(!): what does that mean for, say, .anim files that index by image name....just change the extensions from .png to .eimg


BLEND_STATES SETUP:
-------------------
[NOTE] if no floats values are listed (just an animationName\n) then default to (1.0f / numAnimations) * currentAnimationLoadedCount for the default value to evenly distribute their thresholds
OR: add a distribute boolean at the top of the blend state to indicate how to affect/ignore the values in the file
*/

#endif /* EVIL_ANIMATION_CONTROLLER_MANAGER_H */