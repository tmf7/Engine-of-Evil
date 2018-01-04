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
#ifndef EVIL_RENDERIMAGE_ISOMETRIC_H
#define EVIL_RENDERIMAGE_ISOMETRIC_H

#include "RenderImageBase.h"
#include "Bounds3D.h"

class eGridCell;

//**************************************************
//				eRenderImageIsometric
// data used by eRenderer for draw-order sorting of eGameObjects
// and drawing to an eRenderTarget using isometric projection
//**************************************************
class eRenderImageIsometric : public eRenderImageBase {
private:

	friend class eRenderer;						// directly sets priority, allBehind, and visited

public:

												eRenderImageIsometric(eGameObject * owner, const std::shared_ptr<eImage> & initialImage, const eVec3 & renderBlockSize, int initialImageFrame = 0, const eVec2 & offset = vec2_zero, bool isPlayerSelectable = false);
	virtual									   ~eRenderImageIsometric() override;
	
	void										SetRenderBlockSize(const eVec3 & newSize);
	const eBounds3D &							GetRenderBlock() const;
	const std::vector<eGridCell *> &			Areas() const;

	virtual void								Update() override;
	virtual std::unique_ptr<eComponent>			GetCopy() const	override					{ return std::make_unique<eRenderImageIsometric>(*this); }
	virtual int									GetClassType() const override				{ return CLASS_RENDERIMAGE_ISOMETRIC; }
	virtual bool								IsClassType(int classType) const override	{ 
													if(classType == CLASS_RENDERIMAGE_ISOMETRIC) 
														return true; 
													return eRenderImageBase::IsClassType(classType); 
												}

private:

	void										UpdateRenderBlock();
	void										ClearAreas();
	void										UpdateAreasWorldClipCorners();
	void										UpdateAreasWorldClipArea();

private:

	std::vector<eRenderImageIsometric *>		allBehind;						// topological sort
	std::vector<eGridCell *>					areas;							// the gridcells responsible for drawing *this
	eBounds3D									renderBlock;					// determines draw order of visible images
	bool										visited			= false;		// topological sort
};

//*************
// eRenderImageIsometric::GetRenderBlock
//*************
inline const eBounds3D & eRenderImageIsometric::GetRenderBlock() const {
	return renderBlock;
}

//*************
// eRenderImageIsometric::Areas
//*************
inline const std::vector<eGridCell *> & eRenderImageIsometric::Areas() const {
	return areas;
}

#endif /* EVIL_RENDERIMAGE_ISOMETRIC_H */

