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
//*******************************************************************
//	all engine of evil classes that can have instantiated objects
//	and are designed to be part of an inheritance chain beyond eClass
//*******************************************************************
REGISTER_ENUM(CLASS_CLASS)
REGISTER_ENUM(CLASS_GAMEOBJECT)
REGISTER_ENUM(CLASS_COMPONENT)
REGISTER_ENUM(CLASS_ENTITY)
REGISTER_ENUM(CLASS_TILEIMPL)
REGISTER_ENUM(CLASS_TILE)
REGISTER_ENUM(CLASS_MOVEMENT)
REGISTER_ENUM(CLASS_BOUNDS)
REGISTER_ENUM(CLASS_BOUNDS3D)
REGISTER_ENUM(CLASS_BOX)
REGISTER_ENUM(CLASS_COLLISIONMODEL)
REGISTER_ENUM(CLASS_PLAYER)
REGISTER_ENUM(CLASS_MAP)
REGISTER_ENUM(CLASS_STATENODE)
REGISTER_ENUM(CLASS_ANIMATIONSTATE)
REGISTER_ENUM(CLASS_BLENDSTATE)
REGISTER_ENUM(CLASS_STATETRANSITION)
REGISTER_ENUM(CLASS_ANIMATION)
REGISTER_ENUM(CLASS_ANIMATIONCONTROLLER)
REGISTER_ENUM(CLASS_GAME)

REGISTER_ENUM(CLASS_GRIDINDEX)
REGISTER_ENUM(CLASS_TILEKNOWLEDGE)
REGISTER_ENUM(CLASS_GRIDCELL)
REGISTER_ENUM(CLASS_SPATIALINDEXGRID)
REGISTER_ENUM(CLASS_IMAGE)
REGISTER_ENUM(CLASS_RESOURCE_MANAGER)
REGISTER_ENUM(CLASS_IMAGE_MANAGER)
REGISTER_ENUM(CLASS_ENTITYPREFAB_MANAGER)
REGISTER_ENUM(CLASS_ANIMATION_MANAGER)
REGISTER_ENUM(CLASS_ANIMATIONCONTROLLER_MANAGER)
REGISTER_ENUM(CLASS_RENDERER)
REGISTER_ENUM(CLASS_RENDERTARGET)
REGISTER_ENUM(CLASS_CAMERA)
REGISTER_ENUM(CLASS_CANVAS)
REGISTER_ENUM(CLASS_RENDERIMAGE_BASE)
REGISTER_ENUM(CLASS_RENDERIMAGE_ISOMETRIC)
REGISTER_ENUM(CLASS_INPUT)

REGISTER_ENUM(CLASS_SHERO)				// TODO: allow user to create a separate REGISTER_ENUM list

#if 0
// not designed for inheritance
REGISTER_ENUM(CLASS_VEC2)
REGISTER_ENUM(CLASS_VEC3)
REGISTER_ENUM(CLASS_QUAT)
REGISTER_ENUM(CLASS_DEQUE)
REGISTER_ENUM(CLASS_BINARYHEAP)
REGISTER_ENUM(CLASS_HASHINDEX)

// static classes
REGISTER_ENUM(CLASS_COLLISION)
REGISTER_ENUM(CLASS_SORT)
REGISTER_ENUM(CLASS_MATH)
#endif