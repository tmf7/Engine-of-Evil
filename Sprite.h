#ifndef EVIL_SPRITE_H
#define EVIL_SPRITE_H

#include "Image.h"

// TODO: handle sprite sheets that have multiple different frame sizes (even in a single animation)
// that use alpha values instead of color keys

//*************************
//			eSprite
// Handles animation of image data
//*************************
class eSprite {
public:

					eSprite();

	eImage *		Image();
	void			SetImage(const eImage & image);
	void			NextFrame();
	void			SetAnimation(const int first, const int last, const int frameDelay);
	void			Pause(bool wantPause = true);
	const int		Width() const;
	const int		Height() const;

private:

	eImage			image;
	int				firstFrame;
	int				lastFrame;
	int				frameDelay;
	int				currentFrame;
	int				delayCounter;
	bool			paused;
};

//************
// eSprite::eSprite
//************
inline eSprite::eSprite() 
	: firstFrame(NULL), lastFrame(NULL), frameDelay(NULL),
	  currentFrame(NULL), delayCounter(NULL), paused(true) {
}

//************
// eSprite::Image
// mutable access to sprite's image pixel data
//************
inline eImage * eSprite::Image() {
	return &image;
}

//************
// eSprite::SetImage
//************
inline void eSprite::SetImage(const eImage & image) {
	this->image = image;
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
	return image.IsValid() ? image.Frame().w : -1;
}

//************
// eSprite::Height
// wrapper function for sprite's current frame height
//************
inline const int eSprite::Height() const {
	return image.IsValid() ? image.Frame().h : -1;
}

#endif /* EVIL_SPRITE_H */

