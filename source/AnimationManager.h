#ifndef EVIL_ANIMATION_MANAGER_H
#define EVIL_ANIMATION_MANAGER_H

#include "Definitions.h"
#include "Animation.h"
#include "HashIndex.h"
#include "Renderer.h"

//**********************************
//			eAnimationManager
// Handles all animation allocation and freeing
// DEBUG: --no other object/system should allocate/free animations--
//**********************************
class eAnimationManager {
public:

	bool			Init();
	bool			BatchLoad(const char * animationBatchLoadFile);
	bool			GetAnimation(const char * filename, std::shared_ptr<eAnimation> & result);
	bool			GetAnimation(int animationID, std::shared_ptr<eAnimation> & result);
	bool			LoadAnimation(const char * filename, std::shared_ptr<eAnimation> & result);
	int				GetNumAnimations() const;
	void			Clear();

private:

	std::vector<std::shared_ptr<eAnimation>>		animationList;			// dynamically allocated eAnimation resources
	eHashIndex										animationFilenameHash;	// quick access to animationList
};

//***************
// eAnimationManager::GetNumAnimations
//***************
inline int eAnimationManager::GetNumAnimations() const {
	return animationList.size();
}

#endif /* EVIL_ANIMATION_MANAGER_H */