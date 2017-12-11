/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#include "Bounds.h"
#include "Definitions.h"

//*************
// eBounds::eBounds
// constructs fitted AABB from list of random points
//*************
eBounds::eBounds(const eVec2 * points, const int numPoints) {
	FromPoints(points, numPoints);
}

//*************
// eBounds::FromPoints
// modifies current mins and maxs to from 
// a fitted AABB from a list of random points 
//*************
void eBounds::FromPoints(const eVec2 * points, const int numPoints) {
	static const eVec2 axes[2] = { eVec2(1.0f, 0.0f), eVec2(0.0f, 1.0f) };
	eVec2 mins;
	eVec2 maxs;

	// find the max and min projection coordinate along each axis
	for (int i = 0; i < 2; ++i) {
		float minproj = FLT_MAX;
		float maxproj = -FLT_MAX;
		for (int j = 0; j < numPoints; ++j) {
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
	// 45 degrees CCW about z-axis (not strictly a foreshortening rotation about 2 axes like a normal isometric projection)
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