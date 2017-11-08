#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "CollisionModel.h"

//***********************************************
//				eCamera 
// Mobile 2D Axis-Aligned box for 
// rendering alignment and ViewPort culling
// TODO: inherit from an eGameObject class
// that has optional eCollisionModel, eSprite, 
// eMovment, etc components
//***********************************************
class eCamera : public eClass {
public:

									eCamera();

	void							Think();
	void							Init();
	float							GetZoom() const;
	void							SetZoom(float level);
	bool							Moved() const;
	eCollisionModel &				CollisionModel();
	eVec2							ScreenToWorldPosition(const eVec2 & screenPoint) const;
	eVec2							MouseWorldPosition() const;

	virtual int						GetClassType() const override { return CLASS_CAMERA; }

public:

	// FIXME: load these from engine config file
	static constexpr const float	zoomIncrement = 0.1f;
	static constexpr const float	maxZoom = 2.0f;
	static constexpr const float	minZoom = 0.1f;				
	static constexpr const float	defaultCamSpeed = 10.0f;

private:
	
	eCollisionModel					collisionModel;					
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

//***************
// eCamera::CollisionModel
//***************
inline eCollisionModel & eCamera::CollisionModel() {
	return collisionModel;
}

#endif /* EVIL_CAMERA_H */

