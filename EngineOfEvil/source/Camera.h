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
#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "GameObject.h"

//***********************************************
//				eCamera 
// Mobile 2D Axis-Aligned box for 
// rendering alignment and ViewPort culling
//***********************************************
class eCamera : public eGameObject {
public:

									eCamera();

	void							Think();
	void							Init();
	float							GetZoom() const;
	void							SetZoom(float level);
	bool							Moved() const;
	eVec2							ScreenToWorldPosition(const eVec2 & screenPoint) const;
	eVec2							MouseWorldPosition() const;

	virtual int						GetClassType() const override				{ return CLASS_CAMERA; }
	virtual bool					IsClassType(int classType) const override	{ 
										if(classType == CLASS_CAMERA) 
											return true; 
										return eGameObject::IsClassType(classType); 
									}

public:

	// FIXME: load these from engine config file
	static constexpr const float	zoomIncrement	= 0.1f;
	static constexpr const float	maxZoom			= 2.0f;
	static constexpr const float	minZoom			= 0.1f;				
	static constexpr const float	defaultCamSpeed = 10.0f;

private:
	
	float							camSpeed;
	float							zoomLevel;
	bool							moved;
};	

//***************
// eCamera::GetZoom
//***************
inline float eCamera::GetZoom() const {
	return zoomLevel;
}

//***************
// eCamera::Moved
// DEBUG: includes zoom and translation
//***************
inline bool eCamera::Moved() const {
	return moved;
}

#endif /* EVIL_CAMERA_H */

