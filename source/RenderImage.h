#ifndef EVIL_RENDERIMAGE_H
#define EVIL_RENDERIMAGE_H

#include "Definitions.h"
#include "Bounds.h"
#include "Bounds3D.h"
#include "Sort.h"
#include "Image.h"

//**************************************************
//				eRenderImage
// data used by eRenderer for draw-order sorting
// and drawing to the render target
//**************************************************
class eRenderImage : public eClass {
private:

	friend class eRenderer;		// sets priority, dstRect, lastDrawnTime, allBehind, visited

public:
	
	std::shared_ptr<eImage> &			Image();
	const std::shared_ptr<eImage> &		Image() const;
	void								SetImageFrame(const SDL_Rect & imageFrame);
	const SDL_Rect *					GetImageFrame() const;
	eVec2 &								Origin();
	const eVec2 &						Origin() const;
	eBounds3D &							RenderBlock();
	const eBounds3D &					RenderBlock() const;
	int									GetLayer() const;
	void								SetRenderBlockZFromLayer(const int newLayer);
	void								UpdateLayerFromRenderBlockZ();
	void								UpdateWorldClip();			// DEBUG: only call this after srcRect/ImageFrame has been assigned
	const eBounds &						GetWorldClip() const;

	void								AssignToWorldGrid();

	virtual int							GetClassType() const override { return CLASS_RENDERIMAGE; }

private:

	std::shared_ptr<eImage>				image = nullptr;		// source image (ie texture wrapper)
	const SDL_Rect *					srcRect = nullptr;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect							dstRect;				// SDL consumable cliprect, where on the screen (adjusted with camera position)
																// DEBUG: dimensions relative to srcRect will affect scaling
																// DEBUG: only calculate dstRect from srcRect and origin

	eVec2								origin;					// top-left corner of image using world coordinates (not adjusted with camera position)
	float								priority;				// determined during topological sort, lower priority draws first
	Uint32								layer = MAX_LAYER;		// determines (and determined by) lowest z-depth of renderBlock (see: eSpatialIndexGrid::...ZPosition...) 
	Uint32								lastDrawTime = 0;		// prevent attempts to draw this more than once per frame

//	eClass *							owner;					// eClass (FIXME: eGameObject) using *this

	std::vector<eGridCell *>			areas;					// TODO: the gridcells responsible for drawing *this

// FREEHILL BEGIN 3d topological sort
	eBounds								worldClip;				// dstRect in world space (ie: not adjusted with camera position yet) used for occlusion tests
	eBounds3D							renderBlock;			// determines draw order of visible images
	std::vector<eRenderImage *>			allBehind;				// topological sort
	bool								visited = false;		// topological sort
// FREEHILL END 3d topological sort
};

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
inline eVec2 & eRenderImage::Origin() {
	return origin;
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
// eRenderImage::GetLayer
//*************
inline int eRenderImage::GetLayer() const {
	return layer;
}

//*************
// eRenderImage::UpdateWorldClip
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

#endif /* EVIL_RENDERIMAGE_H */

