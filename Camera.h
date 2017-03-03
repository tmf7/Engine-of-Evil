#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "Entity.h"

class eCamera : public eEntity {
public:

						eCamera();

	virtual void		Think() override;
	void				Init();
	float				GetZoom() const;
	void				SetZoom(float level);

	static constexpr const float zoomIncrement = 0.1f;
	static constexpr const float maxZoom = 2.0f;
	static constexpr const float minZoom = 0.1f;

private:
	
	static constexpr const float	defaultCamSpeed = 10.0f;		// FIXME: 
	float				camSpeed;
	float				zoomLevel;
};	

//***************
// eCamera::eCamera
//***************
inline eCamera::eCamera()
	: camSpeed(defaultCamSpeed) {
}

//***************
// eCamera::GetZoom
//***************
inline float eCamera::GetZoom() const {
	return zoomLevel;
}

#endif /* EVIL_CAMERA_H */

