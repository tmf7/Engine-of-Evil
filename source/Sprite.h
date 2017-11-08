#ifndef EVIL_SPRITE_H
#define EVIL_SPRITE_H

#include "Image.h"

//*************************
//			eSprite
// Handles animation of image data
//*************************
class eSprite : public eClass {
public:

							eSprite();

	bool					Init(const char * filename);
	std::shared_ptr<eImage>	GetImage() const;
	void					SetImage(std::shared_ptr<eImage> & image);
	void					NextFrame();
	void					SetAnimation(const int first, const int last, const int frameDelay);
	void					Pause(bool wantPause = true);
	const SDL_Rect &		GetFrameHack() const;

	virtual int				GetClassType() const override { return CLASS_SPRITE; }

private:

//	typedef std::shared_ptr<eAnimation> Animation_t;
//	std::vector<Animation_t>	animations;		// all source images and their associated sub-frames (clip-rects) to fully animate this "character"

	// TODO: first, last, current may be deprecated due to the circular-link list that Animation provides for animation sequences
	std::shared_ptr<eImage>	currentImage;
	int						firstFrame;
	int						lastFrame;

	int						frameDelay;
	int						currentFrame;
	int						delayCounter;
	bool					paused;

	SDL_Rect				spriteFrameHack;		// FIXME: hack for single-frame non-animated sprite (images themselves dont use frames, they just wrap a texture)

	// experimental
	int						drawOrigin;				// TODO: this will be the world-coordinate where drawing begins (must sync with collisionModel origin, even if offset)
	std::string				name;					// overall name of the sprite relative to its owner "melee_32_sprite"
};

//************
// eSprite::eSprite
//************
inline eSprite::eSprite() 
	: firstFrame(NULL), lastFrame(NULL), frameDelay(NULL), currentImage(nullptr),
	  currentFrame(NULL), delayCounter(NULL), paused(true) {
}

//************
// eSprite::Init
// TODO: parse a sprite def file to load images and their associated frame data
// pass each image's frame data to each individual image (it tracks its own list of subframes)
//************
inline bool eSprite::Init(const char * filename) {
	return false;
}

//************
// eSprite::GetImage
//************
inline std::shared_ptr<eImage> eSprite::GetImage() const {
	return currentImage;
}

//************
// eSprite::SetImage
//************
inline void eSprite::SetImage(std::shared_ptr<eImage> & image) {
	currentImage = image;
	spriteFrameHack = SDL_Rect{ 0, 0, image->GetWidth(), image->GetHeight() };
}

//************
// eSprite::NextFrame
// continues the current state of animation
// must be unpaused to fully animate
//************
inline void eSprite::NextFrame() {
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
// eSprite::SetAnimation
// sets the sprite-specific animation identifiers
//************
inline void eSprite::SetAnimation(const int first, const int last, const int frameDelay) {
	firstFrame = first;
	lastFrame = last;
	this->frameDelay = frameDelay;
	delayCounter = 0;
}

//************
// eSprite::Pause
// stops sprite animation on the currentFrame
//************
inline void eSprite::Pause(bool wantPause) {
	paused = wantPause;
}

//************
// eSprite::GetFrame
// FIXME: hack function to test single-frame non-animated sprite
//************
inline const SDL_Rect & eSprite::GetFrameHack() const {
	return spriteFrameHack;
}

#endif /* EVIL_SPRITE_H */

