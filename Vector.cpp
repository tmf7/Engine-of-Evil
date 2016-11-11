#include "Vector.h"

eVec2 vec2_zero(0.0f, 0.0f);
eVec2 vec2_oneZero(1.0f, 0.0f);
eVec3 vec3_zero(0.0f, 0.0f, 0.0f);
eVec3 vec3_oneZero(1.0f, 0.0f, 0.0f);
eQuat rotationQuat_Z(0.0f, 0.0f, SDL_sinf(DEG2RAD(ROTATION_INCREMENT) / 2.0f), SDL_cosf(DEG2RAD(ROTATION_INCREMENT) / 2.0f));