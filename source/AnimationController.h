#ifndef EVIL_ANIMATION_CONTROLLER_H
#define EVIL_ANIMATION_CONTROLLER_H

#include "Image.h"
#include "Component.h"

//*******************************************
//			eAnimationController
// Handles animation of image data
//*******************************************
class eAnimationController : public eComponent {
public:

							eAnimationController(eGameObject * owner);

	bool					Init(const char * filename);
	std::shared_ptr<eImage>	GetImage() const;
	void					SetImage(std::shared_ptr<eImage> & image);
	void					NextFrame();
	void					SetAnimation(const int first, const int last, const int frameDelay);
	void					Pause(bool wantPause = true);
	const SDL_Rect &		GetFrameHack() const;

	virtual int				GetClassType() const override { return CLASS_ANIMATIONCONTROLLER; }

private:

//	typedef std::shared_ptr<eAnimation> Animation_t;
//	std::vector<eAnimation>	animations;		// all source images and their associated sub-frames (clip-rects) to fully animate an eRenderImage

	// TODO: first, last, current will be deprecated by the sequence that eAnimation will provide
	std::shared_ptr<eImage>	currentImage	= nullptr;
	int						firstFrame		= 0;
	int						lastFrame		= 0;

	int						frameDelay		= 0;
	int						currentFrame	= 0;
	int						delayCounter	= 0;
	bool					paused			= true;

	SDL_Rect				spriteFrameHack;		// FIXME: hack for single-frame non-animated renderImage (images themselves dont use frames, they just wrap a texture)

	// experimental
	std::string				name;					// overall name of the animationController relative to its owner "melee_32_controller"
};

//************
// eAnimationController::eAnimationController
//************
inline eAnimationController::eAnimationController(eGameObject * owner) {
	this->owner = owner;
}

//************
// eAnimationController::Init
//************
inline bool eAnimationController::Init(const char * filename) {
	// TODO: implement
	return false;
}

//************
// eAnimationController::GetImage
//************
inline std::shared_ptr<eImage> eAnimationController::GetImage() const {
	return currentImage;
}

//************
// eAnimationController::SetImage
//************
inline void eAnimationController::SetImage(std::shared_ptr<eImage> & image) {
	currentImage = image;
	spriteFrameHack = SDL_Rect{ 0, 0, image->GetWidth(), image->GetHeight() };
}

//************
// eAnimationController::NextFrame
// continues the current state of animation
// must be unpaused to fully animate
//************
inline void eAnimationController::NextFrame() {
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
// eAnimationController::SetAnimation
// sets the eGameobject instance-specific animation identifiers
//************
inline void eAnimationController::SetAnimation(const int first, const int last, const int frameDelay) {
	firstFrame = first;
	lastFrame = last;
	this->frameDelay = frameDelay;
	delayCounter = 0;
}

//************
// eAnimationController::Pause
// stops animation on the currentFrame
//************
inline void eAnimationController::Pause(bool wantPause) {
	paused = wantPause;
}

//************
// eAnimationController::GetFrame
// FIXME: hack function to test single-frame non-animated renderImage
//************
inline const SDL_Rect & eAnimationController::GetFrameHack() const {
	return spriteFrameHack;
}

#endif /* EVIL_ANIMATION_CONTROLLER_H */

