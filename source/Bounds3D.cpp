#include "Bounds3D.h"
#include "Definitions.h"

//*************
// eBounds3D::FromPoints
// fitted AABB from a list of random points 
//*************
void eBounds3D::FromPoints(const eVec3 * points, const int numPoints) {
	static const eVec3 axes[3] = { eVec3(1.0f, 0.0f, 0.0f), eVec3(0.0f, 1.0f, 0.0f), eVec3(0.0f, 0.0f, 1.0f) };
	eVec3 mins;
	eVec3 maxs;

	// find the max and min projection coordinate along each axis
	for (int i = 0; i < 3; i++) {
		float minproj = FLT_MAX;
		float maxproj = -FLT_MAX;
		for (int j = 0; j < numPoints; j++) {
			float proj = points[j] * axes[i];
			if (proj < minproj) {
				minproj = proj;
				mins[i] = points[j][i];
			}
			if (proj > maxproj) {
				maxproj = proj;
				maxs[i] = points[j][i];
			}
		}
	}
	bounds[0] = mins;
	bounds[1] = maxs;
}

//*************
// eBounds3D::FromIsometricRotation
// DEBUG: must use local-space bounds for accurate size and position
//*************
void eBounds3D::FromIsometricRotation(const eVec3 & origin) {
	// 45 degrees CCW about z-axis (not strictly a foreshortening rotation about 2 axes like a normal isometric projection)
	static const float cos45 = SDL_sqrtf(2) * 0.5f;
	static const eVec3 R_CCW[3] = {	eVec3(cos45, cos45, 0.0f),
									eVec3(-cos45, cos45, 0.0f),
									vec3_zero					};

	eVec3 center, extents;
	eVec3 rotatedCenter, rotatedExtents;
	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;

	for (int i = 0; i < 3; i++) {
		rotatedExtents[i] = abs(extents[0] * R_CCW[0][i]) +
							abs(extents[1] * R_CCW[1][i]);

		rotatedCenter[i] = origin[i];
		rotatedCenter[i] +=	(center[0] * R_CCW[0][i]) +
							(center[1] * R_CCW[1][i]);
	}
	bounds[0] = rotatedCenter - rotatedExtents;
	bounds[1] = rotatedCenter + rotatedExtents;
}