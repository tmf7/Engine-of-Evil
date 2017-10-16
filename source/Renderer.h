#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Bounds3D.h"
#include "Sort.h"
#include "Image.h"

// eRenderType_t
typedef enum {
	RENDERTYPE_STATIC,
	RENDERTYPE_DYNAMIC
} eRenderType_t;

// renderImage_t
typedef struct renderImage_s {
	std::shared_ptr<eImage>		image;			// source image (ie texture wrapper)
	const SDL_Rect *			srcRect;		// what part of the source image to draw (nullptr for all of it)
	SDL_Rect					dstRect;		// SDL consumable cliprect, where on the screen (adjusted with camera position)
												// DEBUG: dimensions relative to srcRect will affect scaling
												// DEBUG: only calculate dstRect from srcRect and origin
	eVec2						origin;			// top-left corner of image using world coordinates (not adjusted with camera position)
	float						priority;		// combination of layer and meta-z coordinate **during AddToRenderPool**, lower priority draws first
	Uint32						layer;			// the primary draw sorting criteria
	Uint32						lastDrawTime;	// prevent multiple attempts to draw this twice in one frame

//	eEntity *					owner;			// entity using this renderimage, for secondary renderPool sort if priority causes flicker

// FREEHILL BEGIN 3d quicksort test
	eBounds						worldClip;		// dstRect in world space (ie: not adjusted with camera position yet) used for occlusion tests
	eBounds3D					renderBlock;	// determines draw order of visible images
	std::vector<renderImage_s *> allBehind;		// topological sort
	bool						visited;		// topological sort
// FREEHILL END 3d quicksort test

								renderImage_s()
									: image(nullptr),
									  srcRect(nullptr),
									  layer(MAX_LAYER),
									  lastDrawTime(0) {};
	
								renderImage_s(std::shared_ptr<eImage> & image, const SDL_Rect * srcRect, const eVec2 & origin, const Uint8 layer)
									: image(image),
									  srcRect(srcRect), 
									  origin(origin),
									  layer(layer) {
								};

	void	SetDrawnTime(Uint32 drawTime)	{ lastDrawTime = drawTime; }
	void	SetLayer(const int layer)		{ this->layer = layer; }
	Uint32	GetLayer() const				{ return layer; }
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
// DynamicPool draws to a scalable target that typically tracks with the camera
// StaticPool draws so a non-scalable target that typically draws to screen coordinates
// DEBUG: RENDERTYPE_STATIC items must always be drawn last 
// otherwise they will be overwritten by the scalable target
// and its RENDERTYPE_DYANMIC items
//****************************************************
class eRenderer {
public:

						eRenderer();
						
	bool				Init();
	void				Free() const;
	void				Clear() const;
	void				Show() const;
	SDL_Rect			ViewArea() const;

	SDL_Renderer *		GetSDLRenderer() const;
	SDL_Window *		GetWindow() const;

	void				AddToRenderPool(renderImage_t * renderImage, bool dynamic, bool reprioritize = false);
	void				FlushDynamicPool();
	void				FlushStaticPool();

	void				DrawOutlineText(const char * text, eVec2 & point, const SDL_Color & color, bool constText, bool dynamic);
	void				DrawImage(renderImage_t * renderImage) const;

	void				DrawIsometricPrism(const SDL_Color & color, eBounds3D rect, bool dynamic) const;
	void				DrawIsometricRect(const SDL_Color & color, eBounds rect, bool dynamic) const;
	void				DrawCartesianRect(const SDL_Color & color, eBounds rect, bool fill, bool dynamic) const;

	static void			TopologicalDrawDepthSort(const std::vector<renderImage_t *> & renderImagePool);

private:
	
	static void			VisitTopologicalNode(renderImage_t * renderImage);

private:

	static int							globalDrawDepth;				// for eRenderer::TopologicalDrawDepthSort
	static const int					defaultRenderCapacity = 1024;	// maximum number of items to draw using each render pool
	std::vector<renderImage_t *>		staticPoolInserts;				// minimize priority re-calculations	
	std::vector<renderImage_t *>		dynamicPoolInserts;				// minimize priority re-calculations
	std::vector<renderImage_t *>		staticPool;					
	std::vector<renderImage_t *>		dynamicPool;				

	SDL_Window *						window;
	SDL_Renderer *						internal_renderer;
	SDL_Texture *						scalableTarget;
	TTF_Font *							font;
};

// utility colors
extern const SDL_Color clearColor;
extern const SDL_Color blackColor;
extern const SDL_Color greyColor_trans;
extern const SDL_Color greenColor;
extern const SDL_Color redColor;
extern const SDL_Color blueColor;
extern const SDL_Color pinkColor;
extern const SDL_Color lightBlueColor;
extern const SDL_Color yellowColor;

//***************
// eRenderer::eRenderer
//***************
inline eRenderer::eRenderer() {
	staticPool.reserve(defaultRenderCapacity);
	dynamicPool.reserve(defaultRenderCapacity);
}

//***************
// eRenderer::Clear
//***************
inline void eRenderer::Clear() const {
	SDL_SetRenderTarget(internal_renderer, scalableTarget);
	SDL_RenderClear(internal_renderer);
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_RenderClear(internal_renderer);
}

//***************
// eRenderer::Show
// updates the visible screen area
//***************
inline void eRenderer::Show() const {
	SDL_RenderPresent(internal_renderer);
}

//***************
// eRenderer::ViewArea
//***************
inline SDL_Rect eRenderer::ViewArea() const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	return viewArea;
}

//***************
// eRenderer::GetRenderer
//***************
inline SDL_Renderer * eRenderer::GetSDLRenderer() const {
	return internal_renderer;
}

//*****************
// eRenderer::GetWindow
//*****************
inline SDL_Window * eRenderer::GetWindow() const {
	return window;
}

#endif /* EVIL_RENDERER_H */
