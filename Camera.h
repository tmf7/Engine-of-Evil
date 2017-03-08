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
	eCollisionModel &				CollisionModel();

	static constexpr const float	zoomIncrement = 0.1f;
	static constexpr const float	maxZoom = 2.0f;
	static constexpr const float	minZoom = 0.1f;

private:
	
	eCollisionModel					collisionModel;					

	static constexpr const float	defaultCamSpeed = 10.0f;		// FIXME: load from file?
	float							camSpeed;
	float							zoomLevel;
};	

//***************
// eCamera::eCamera
//***************
inline eCamera::eCamera()
	: camSpeed(defaultCamSpeed) {
	collisionModel.SetActive(false);	// DEBUG: does not participate in normal collision detection (eg UpdateAreas doesn't happen)
}

//***************
// eCamera::GetZoom
//***************
inline float eCamera::GetZoom() const {
	return zoomLevel;
}

//***************
// eCamera::CollisionModel
//***************
inline eCollisionModel & eCamera::CollisionModel() {
	return collisionModel;
}

#endif /* EVIL_CAMERA_H */

