#include "Bounds.h"
#include "Definitions.h"

//*************
// eBounds::FromPoints
// fitted AABB from a list of random points 
//*************
void eBounds::FromPoints(const eVec2 * points, const int numPoints) {
	static const eVec2 axes[2] = { eVec2(1.0f, 0.0f), eVec2(0.0f, 1.0f) };
	eVec2 mins;
	eVec2 maxs;

	// find the max and min projection coordinate along each axis
	for (int i = 0; i < 2; i++) {
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
// eBounds::FromIsometricRotation
// DEBUG: must use local-space bounds for accurate size and position
//*************
void eBounds::FromIsometricRotation(const eVec2 & origin) {
	// 45 degrees CCW about z-axis
	static const float cos45 = SDL_sqrtf(2) * 0.5f;
	static const eVec2 R_CCW[2] = {	eVec2(cos45, cos45),
									eVec2(-cos45, cos45)};

	eVec2 center, extents;
	eVec2 rotatedCenter, rotatedExtents;
	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;

	for (int i = 0; i < 2; i++) {
		rotatedExtents[i] = abs(extents[0] * R_CCW[0][i]) +
							abs(extents[1] * R_CCW[1][i]);

		rotatedCenter[i] = origin[i];
		rotatedCenter[i] +=	(center[0] * R_CCW[0][i]) +
							(center[1] * R_CCW[1][i]);
	}
	bounds[0] = rotatedCenter - rotatedExtents;
	bounds[1] = rotatedCenter + rotatedExtents;
}