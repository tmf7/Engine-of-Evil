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
#include "Box.h"

//*************
// eBox::eBox
// DEBUG: assumes all three points are corners of a rectangle
// with the 0th point common to two edges
// and the 1st and 2nd points being on opposite corners
//*************
eBox::eBox(const eVec2 points[3]) {
	eVec2 xAxis = points[1] - points[0];
	eVec2 yAxis = points[2] - points[0];
	float xLength = xAxis.Length();
	float yLength = yAxis.Length();
	float invLengthX = xLength == 0.0f ? 0.0f : 1.0f / xLength;
	float invLengthY = yLength == 0.0f ? 0.0f : 1.0f / yLength;

	extents.x = xLength * 0.5f;
	extents.y = yLength * 0.5f;
	axes[0] = xAxis * invLengthX;
	axes[1] = yAxis * invLengthY;
	center = (points[1] + points[2]) * 0.5f;
}

//*************
// eBox::eBox
// DEBUG: edges must be perpendicular
// and corner must be their point of intersection
//*************
eBox::eBox(const eVec2 & corner, const eVec2 edges[2]) {
	float xLength = edges[0].Length();
	float yLength = edges[1].Length();
	float invLengthX = xLength == 0.0f ? 0.0f : 1.0f / xLength;
	float invLengthY = yLength == 0.0f ? 0.0f : 1.0f / yLength;

	extents.x = xLength * 0.5f;
	extents.y = yLength * 0.5f;
	axes[0] = edges[0] * invLengthX;
	axes[1] = edges[1] * invLengthY;
	center = corner + (edges[0] + edges[1]) * 0.5f;
}