#ifndef EVIL_SPRITE_H
#define EVIL_SPRITE_H

#include "Image.h"

// TODO: handle sprite sheets that have multiple different frame sizes (even in a single animation)
// that use alpha values instead of color keys

//*************************
//			Sprite
// Handles animation of image data
//*************************
class Sprite {
public:
					Sprite();

	Image *			GetImage() const;
	void			SetImage(Image * image);
	void			NextFrame();
	void			SetAnimation(const int first, const int last, const int frameDelay);
	void			Pause(bool on = true);
	const int		Width() const;
	const int		Height() const;

private:

	Image *			image;
	int				firstFrame;
	int				lastFrame;
	int				frameDelay;
	int				currentFrame;
	int				delayCounter;
	bool			paused;
};

//************
// Sprite::Sprite
//************
inline Sprite::Sprite() : firstFrame(NULL), lastFrame(NULL), frameDelay(NULL),
							currentFrame(NULL), delayCounter(NULL), image(NULL), paused(true) {
}

//************
// Sprite::GetImage
// direct access to sprite's image pixel data
//************
inline Image * Sprite::GetImage() const {
	return image;
}

//************
// Sprite::SetImage
//************
inline void Sprite::SetImage(Image * image) {
	this->image = image;
}

//************
// Sprite::NextFrame
// continues the current state of animation
// must be unpaused to fully animate
//************
inline void Sprite::NextFrame() {
	
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
// Sprite::SetAnimation
// sets the sprite-specific animation identifiers
//************
inline void Sprite::SetAnimation(const int first, const int last, const int frameDelay) {
	firstFrame = first;
	lastFrame = last;
	this->frameDelay = frameDelay;
	delayCounter = 0;
}

//************
// Sprite::Pause
// stops sprite animation on the currentFrame
//************
inline void Sprite::Pause(bool on) {
	paused = on;
}

//************
// Sprite::Width
// wrapper function for sprite's current frame width
//************
inline const int Sprite::Width() const {
	return image->IsValid() ? image->Frame()->w : -1;
}

//************
// Sprite::Height
// wrapper function for sprite's current frame height
//************
inline const int Sprite::Height() const {
	return image->IsValid() ? image->Frame()->h : -1;
}

#endif /* EVIL_SPRITE_H */

