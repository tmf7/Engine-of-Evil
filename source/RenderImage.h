#ifndef EVIL_RENDERIMAGE_H
#define EVIL_RENDERIMAGE_H

#include "Definitions.h"
#include "Bounds.h"
#include "Bounds3D.h"
#include "Sort.h"
#include "Image.h"
#include "Component.h"

class eGridCell;

//**************************************************
//				eRenderImage
// data used by eRenderer for draw-order sorting
// and drawing to the render target
//**************************************************
class eRenderImage : public eComponent {
private:

	friend class eRenderer;				// directly sets dstRect, priority, lastDrawnTime, allBehind, visited (no other accessors outside *this)

public:

										eRenderImage(eGameObject * owner);
	virtual								~eRenderImage() override;
	
	std::shared_ptr<eImage> &			Image();
	const std::shared_ptr<eImage> &		Image() const;
	void								SetImageFrame(const SDL_Rect & imageFrame);
	const SDL_Rect *					GetImageFrame() const;
	void								SetOrigin(const eVec2 & newOrigin);
	const eVec2 &						Origin() const;
	eBounds3D &							RenderBlock();
	const eBounds3D &					RenderBlock() const;
	void								SnapRenderBlockToLayer();
	const eBounds &						GetWorldClip() const;
	const std::vector<eGridCell *> &	Areas() const;
	void								SetIsSelectable(bool isSelectable);
	bool								IsSelectable() const;

	virtual int							GetClassType() const override { return CLASS_RENDERIMAGE; }

private:

	void								UpdateWorldClip();
	void								ClearAreas();
	void								UpdateAreasWorldClipCorners();
	void								UpdateAreasWorldClipArea();

private:

	std::shared_ptr<eImage>				image			= nullptr;		// source image (ie texture wrapper)
	const SDL_Rect *					srcRect			= nullptr;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect							dstRect;						// SDL consumable cliprect, where on the screen (adjusted with camera position)
	eVec2								origin;							// top-left corner of image using world coordinates (not adjusted with camera position)
	eVec2								oldOrigin;						// minimizes number of UpdateAreas calls for non-static eGameObjects that aren't moving
	float								priority;						// determined during topological sort, lower priority draws first
	Uint32								lastDrawTime	= 0;			// prevent attempts to draw this more than once per frame
	std::vector<eGridCell *>			areas;							// the gridcells responsible for drawing *this
	bool								isSelectable	= false;		// if this should added to all eGridCells its worldClip overlaps, or just its corners

// FREEHILL BEGIN 3d topological sort
	eBounds								worldClip;						// dstRect in world space (ie: not adjusted with camera position yet) used for occlusion tests
	eBounds3D							renderBlock;					// determines draw order of visible images
	std::vector<eRenderImage *>			allBehind;						// topological sort
	bool								visited			= false;		// topological sort
// FREEHILL END 3d topological sort
};

//*************
// eRenderImage::eRenderImage
//*************
inline eRenderImage::eRenderImage(eGameObject * owner) {
	this->owner = owner;
}

//*************
// eRenderImage::Image
//*************
inline std::shared_ptr<eImage> & eRenderImage::Image() {
	return image;
}

//*************
// eRenderImage::Image
//*************
inline const std::shared_ptr<eImage> & eRenderImage::Image() const {
	return image;
}

//*************
// eRenderImage::ImageFrame
//*************
inline void eRenderImage::SetImageFrame(const SDL_Rect & imageFrame) {
	srcRect = &imageFrame;
}

//*************
// eRenderImage::ImageFrame
//*************
inline const SDL_Rect * eRenderImage::GetImageFrame() const {
	return srcRect;
}

//*************
// eRenderImage::Origin
//*************
inline const eVec2 & eRenderImage::Origin() const {
	return origin;
}

//*************
// eRenderImage::RenderBlock
//*************
inline eBounds3D & eRenderImage::RenderBlock() {
	return renderBlock;
}

//*************
// eRenderImage::RenderBlock
//*************
inline const eBounds3D & eRenderImage::RenderBlock() const {
	return renderBlock;
}

//*************
// eRenderImage::UpdateWorldClip
// DEBUG: only call this after ImageFrame has been assigned
//*************
inline void eRenderImage::UpdateWorldClip() {
	worldClip = eBounds(origin, origin + eVec2((float)srcRect->w, (float)srcRect->h));
}

//*************
// eRenderImage::GetWorldClip
//*************
inline const eBounds & eRenderImage::GetWorldClip() const {
	return worldClip;
}

//*************
// eRenderImage::Areas
//*************
inline const std::vector<eGridCell *> & eRenderImage::Areas() const {
	return areas;
}

//*************
// eRenderImage::SetIsSelectable
//*************
inline void eRenderImage::SetIsSelectable(bool isSelectable) {
	this->isSelectable = isSelectable;
}

//*************
// eRenderImage::IsSelectable
//*************
inline bool eRenderImage::IsSelectable() const {
	return isSelectable;
}

#endif /* EVIL_RENDERIMAGE_H */

