#ifndef EVIL_SPRITE_H
#define EVIL_SPRITE_H

#include "Image.h"
#include "HashIndex.h"

// TODO: handle sprite sheets that have multiple different frame sizes (even in a single animation)
// that use alpha values instead of color keys

//*************************
//			eSprite
// Handles animation of image data
//*************************
class eSprite {
public:

							eSprite();

	bool					Init(const char * filename);
	eImage *				Image();
	void					SetImage(eImage * image);
	void					NextFrame();
	void					SetAnimation(const int first, const int last, const int frameDelay);
	void					Pause(bool wantPause = true);
	const int				Width() const;
	const int				Height() const;

private:

	eHashIndex				imageHash;			// to quickly lookup an animation sheet by name
	std::vector<eImage>		images;				// all animation sheets used by this sprite
	eImage *				currentImage;		// currently active animation sheet
	int						firstFrame;
	int						lastFrame;
	int						frameDelay;
	int						currentFrame;
	int						delayCounter;
	bool					paused;

	// experimental
	int						drawOrigin;
	std::string				name;
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
// eSprite::Image
// mutable access to sprite's current image pixel data
//************
inline eImage * eSprite::Image() {
	return currentImage;
}

//************
// eSprite::SetImage
//************
inline void eSprite::SetImage(eImage * image) {
	currentImage = image;
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
// eSprite::Width
// wrapper function for sprite's current frame width
//************
inline const int eSprite::Width() const {
	return currentImage->IsValid() ? currentImage->Frame().w : -1;
}

//************
// eSprite::Height
// wrapper function for sprite's current frame height
//************
inline const int eSprite::Height() const {
	return currentImage->IsValid() ? currentImage->Frame().h : -1;
}

#endif /* EVIL_SPRITE_H */

