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
#include "Class.h"

#ifndef EVIL_CANVAS_H
#define EVIL_CANVAS_H

//***********************************************
//				eCanvas
// Draws eRenderImageBase objects to either of
// two rendering targets: a main and a debug.
// The debug target draws on top of the main target.
// DEBUG: Use eRenderer to draw on an eCanvas
// and output to a window/rendering context,
// and to ensure eRenderTargets get cleared properly
// TODO: this can be a base class for the three types of eCanvas
// screen overlay, camera overlay, world-space
//***********************************************
class eCanvas : public eClass {

};

#endif /* EVIL_CANVAS_H */