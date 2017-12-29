#ifndef EVIL_RENDERIMAGE_BASE_H
#define EVIL_RENDERIMAGE_BASE_H

#include "Definitions.h"
#include "Bounds.h"
#include "Image.h"

class eRenderTarget;

// TODO: make an eRenderImageBase class that eRenderImage derives from (to become eRenderImageIsometric) and that eButton and eCanvas use,
// such that eRenderer uses DrawImage(eRenderImageBase *) instead of DrawImage(eRenderImageIsometric *)
// PROBLEM: how can an eCanvas draw an eRenderImageBase if it's not part of its UIElements list? (make a eRenderImageUI that derives from eUIElement which adds eCanvas * owner...and nothing else???)

// TODO: eCanvas is similar to eCamera, but without the draw-order sorting that preserves the isometric illusion...and eCanvas can be set to screenspace-overlay, camaeraspace-overlay, worldspace

// TODO: give eCanvas a list of UI elements to draw in order (no sorting) 
// (eg: buttons, plain images, text boxes[?]...all of which may need scaling...eUIElement class w/scale and...offset w/in canvas...and if collision tests occur against their rects)
class eRenderImageBase : public eClass {
private:

	friend class eRenderer;				// directly sets dstRect, priority, lastDrawnTime (no other accessors outside *this)

public:


	std::shared_ptr<eImage> &					Image();
	const std::shared_ptr<eImage> &				GetImage() const;
	void										SetImage(int imageManagerIndex);
	void										SetImageFrame(int subframeIndex);
	const SDL_Rect *							GetImageFrame() const;
	SDL_Rect									GetOverlapImageFrame(const eBounds & otherWorldClip) const;

	void										SetOrigin(const eVec2 & newOrigin);
	const eVec2 &								Origin() const;
	const eBounds &								GetWorldClip() const;

	virtual void								Update() override;
	virtual int									GetClassType() const override				{ return CLASS_RENDERIMAGE; }
	virtual bool								IsClassType(int classType) const override	{ 
													if(classType == CLASS_RENDERIMAGE) 
														return true; 
													return eClass::IsClassType(classType); 
												}

private:

	void										UpdateWorldClip();


private:

	std::vector<eRenderTarget *>				drawnTo;						// prevent attempts to draw this more than once per renderTarget per frame
	std::shared_ptr<eImage>						image			= nullptr;		// source image (ie texture wrapper)

	eBounds										worldClip;						// dstRect in world space (ie: not adjusted with camera position yet) used for occlusion tests
	const SDL_Rect *							srcRect			= nullptr;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect									dstRect;						// SDL consumable cliprect, where on the screen (adjusted with camera position)


	eVec2										origin;							// top-left corner of image using world coordinates (not adjusted with any eRenderTarget position)
	eVec2										oldOrigin;						// minimizes number of UpdateAreas calls for non-static eGameObjects that aren't moving
	float										priority;						// lower priority draws first to an eRenderTarget
	Uint32										lastDrawnTime	= 0;			// allows the drawnTo vector to be cleared before *this is drawn the first time during a frame
};

#endif /* EVIL_RENDERIMAGE_BASE_H */