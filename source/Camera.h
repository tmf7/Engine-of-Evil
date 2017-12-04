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

