#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "CollisionModel.h"

class eCamera {
public:

									eCamera();

	void							Think();
	void							Init();
	float							GetZoom() const;
	void							SetZoom(float level);
	bool							Moved() const;
	eCollisionModel &				CollisionModel();

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
// eCamera::eCamera
//***************
inline eCamera::eCamera()
	: camSpeed(defaultCamSpeed) {
	collisionModel.SetActive(false);	// DEBUG: does not participate in normal collision detection (ie UpdateAreas doesn't happen)
}

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

