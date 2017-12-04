#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include "RenderImage.h"

// eRenderType_t
typedef enum {
	RENDERTYPE_STATIC,
	RENDERTYPE_DYNAMIC
} eRenderType_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
// CameraPool draws to a scalable target that typically tracks with the camera
// OverlayPool draws so a non-scalable target that typically draws to screen coordinates
// DEBUG: RENDERTYPE_STATIC items must always be drawn last 
// otherwise they will be overwritten by the scalable target
// and its RENDERTYPE_DYANMIC items
//***************************************************
class eRenderer : public eClass {
public:

						eRenderer();
						
	bool				Init();
	void				Free() const;
	void				Clear() const;
	void				Show() const;
	SDL_Rect			ViewArea() const;

	SDL_Renderer *		GetSDLRenderer() const;
	SDL_Window *		GetWindow() const;

	void				AddToCameraRenderPool(eRenderImage * renderImage);
	void				AddToOverlayRenderPool(eRenderImage * renderImage);
	void				FlushCameraPool();
	void				FlushOverlayPool();

	void				DrawOutlineText(const char * text, eVec2 & point, const SDL_Color & color, bool constText, bool dynamic);
	void				DrawImage(eRenderImage * renderImage, eRenderType_t renderType) const;
	void				DrawLines(const SDL_Color & color, std::vector<eVec2> points, bool dynamic) const;
	void				DrawIsometricPrism(const SDL_Color & color, const eBounds3D & rect, bool dynamic) const;
	void				DrawIsometricRect(const SDL_Color & color, const eBounds & rect, bool dynamic) const;
	void				DrawCartesianRect(const SDL_Color & color, const eBounds & rect, bool fill, bool dynamic) const;

	virtual int			GetClassType() const override				{ return CLASS_RENDERER; }
	virtual bool		IsClassType(int classType) const override	{ 
							if(classType == CLASS_RENDERER) 
								return true; 
							return eClass::IsClassType(classType); 
						}

	static void			TopologicalDrawDepthSort(const std::vector<eRenderImage *> & renderImagePool);

private:
	
	static void			VisitTopologicalNode(eRenderImage * renderImage);

private:

	static int							globalDrawDepth;				// for eRenderer::TopologicalDrawDepthSort
	static const int					defaultRenderCapacity = 1024;	// maximum number of items to draw using each render pool
	std::vector<eRenderImage *>			overlayPoolInserts;				// minimize priority re-calculations	
	std::vector<eRenderImage *>			cameraPoolInserts;				// minimize priority re-calculations
	std::vector<eRenderImage *>			overlayPool;					// images static to the screen regardless of camera position
	std::vector<eRenderImage *>			cameraPool;						// game-world that moves and scales with the camera

	SDL_Window *						window;
	SDL_Renderer *						internal_renderer;

	// swappable render targets
	// DEBUG: default SDL rendertarget (NULL) is the window texture (ie the "overlayTarget")
	// and uses draw-order sorting based on eRenderImage::renderBlock
	SDL_Texture *						scalableTarget;					// move and scale with the camera, with draw-order sorting based on eRenderImage::renderBlock		
	SDL_Texture *						debugCameraTarget;				// move and scale with the camera, but draw last, without draw-order sorting
	SDL_Texture *						debugOverlayTarget;				// draw straight on the window (no camera adjustment), last, without draw-order sorting
	
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
	overlayPool.reserve(defaultRenderCapacity);
	cameraPool.reserve(defaultRenderCapacity);
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
