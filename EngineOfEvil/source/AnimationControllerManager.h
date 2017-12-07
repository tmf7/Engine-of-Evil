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

	virtual int								GetClassType() const override				{ return CLASS_ANIMATIONCONTROLLER_MANAGER; }
	virtual bool							IsClassType(int classType) const override	{ 
												if(classType == CLASS_ANIMATIONCONTROLLER_MANAGER) 
													return true; 
												return eResourceManager<eAnimationController>::IsClassType(classType); 
											}
};

#endif /* EVIL_ANIMATION_CONTROLLER_MANAGER_H */