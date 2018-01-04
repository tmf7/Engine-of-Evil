/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_CANVAS_H
#define EVIL_CANVAS_H

#include "RenderTarget.h"
#include "RenderImageBase.h"

//***********************************************
//				eCanvas 
// Immobile 2D Axis-Aligned Orthographic box 
// that draws eRenderImageBase objects to
// its eRenderTarget texture during eRenderer::Flush
//***********************************************
class eCanvas : public eRenderTarget {
public:

	friend class eRenderer;

public:

	void											Configure(const eVec2 & size, const eVec2 & worldPosition, const eVec2 & scale = vec2_one);
	bool											AddToRenderPool(eRenderImageBase * renderImage);
	void											ClearRenderPools();

	virtual int										GetClassType() const override				{ return CLASS_CANVAS; }
	virtual bool									IsClassType(int classType) const override	{ 
														if(classType == CLASS_CANVAS) 
															return true; 
														return eRenderTarget::IsClassType(classType); 
													}
private:
	
	std::vector<eRenderImageBase *>					dynamicPool;				// dynamic eGameObjects to draw, minimizes priority re-calculations of dynamic vs. static eGameObjects
	std::vector<eRenderImageBase *>					staticPool;					// static eGameObjects that scale with this eCanvas
};	

#endif /* EVIL_CAMERA_H */
