#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "Entity.h"
#include "Vector.h"

class eCamera : public eEntity {
public:
					
						eCamera();

	virtual void		Think() override;
	void				Init();
	const eVec2 &		GetAbsBounds(bool minMax = false) const;
};	

//***************
// eCamera::eCamera
//***************
inline eCamera::eCamera() {
	speed = 10.0f;
}

//***************
// eCamera::GetAbsBounds
// minMax == false returns absBounds mins, true returns maxs
//***************
inline const eVec2 & eCamera::GetAbsBounds(bool minMax) const {
	return minMax ? absBounds[1] : absBounds[0];
}

#endif /* EVIL_CAMERA_H */

