#ifndef EVIL_ANIMATION_MANAGER_H
#define EVIL_ANIMATION_MANAGER_H

#include "Animation.h"
#include "ResourceManager.h"

//******************************************
//			eAnimationManager
// Handles all entity prefab allocation and freeing
// see also: eResourceManager template
//******************************************
class eAnimationManager : public eResourceManager<eAnimation> {
public:

	virtual bool							Init() override;
	virtual bool							LoadAndGet(const char * resourceFilename, std::shared_ptr<eAnimation> & result) override;

	virtual int								GetClassType() const override { return CLASS_ANIMATION_MANAGER; }
};

#endif  /* EVIL_ANIMATION_MANAGER_H */