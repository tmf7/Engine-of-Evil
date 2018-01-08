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
#ifndef EVIL_GAME_LOCAL_H
#define EVIL_GAME_LOCAL_H

#include "Game.h"
#include "Player.h"
#include "Map.h"
#include "Camera.h"

//*************************************************
//					eGameLocal
// test for user-defined extension of eGame base class
//*************************************************
class eGameLocal : public evil::eGame {
public:

	virtual bool									Init() override;
	virtual void									Update() override;
	virtual void									Shutdown() override		{}

private:

	evil::eMap										map;				// FIXME(?): one map instance per game, use eMap::LoadMap/UnloadMap as needed
	evil::eCamera									camera;				// DEBUG: the main viewport camera
	ePlayer											player;

};

extern eGameLocal gameLocal;

#endif /* EVIL_GAME_LOCAL_H */