#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "Entity.h"
#include "Vector.h"

class eCamera : public eEntity {
public:
					
						eCamera();

	virtual void		Think() override;
	void				Init();
	float				GetZoom() const;
	void				SetZoom(float level);
	const eVec2 &		GetAbsBounds(bool minMax = false) const;

	static constexpr const float zoomIncrement = 0.1f;
	static constexpr const float maxZoom = 2.0f;
	static constexpr const float minZoom = 0.1f;

private:

	float				zoomLevel;
};	

//***************
// eCamera::eCamera
//***************
inline eCamera::eCamera() {
	speed = 10.0f;
}

//***************
// eCamera::GetZoom
//***************
inline float eCamera::GetZoom() const {
	return zoomLevel;
}

//***************
// eCamera::GetAbsBounds
// minMax == false returns absBounds mins, true returns maxs
//***************
inline const eVec2 & eCamera::GetAbsBounds(bool minMax) const {
	return minMax ? absBounds[1] : absBounds[0];
}

#endif /* EVIL_CAMERA_H */

