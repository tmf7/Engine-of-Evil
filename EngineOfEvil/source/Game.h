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
#ifndef EVIL_GAME_H
#define EVIL_GAME_H

#include "ImageManager.h"
#include "AnimationManager.h"
#include "AnimationControllerManager.h"
#include "EntityPrefabManager.h"
#include "Input.h"
#include "Audio.h"

//*************************************************
//					eGame
// handles all sub-system loading, initialization, and unloading
//*************************************************
class eGame : public eClass {
public:

	struct {
		bool	GOAL_WAYPOINTS		= true;
		bool	TRAIL_WAYPOINTS		= false;
		bool	COLLISION			= false;
		bool	RENDERBLOCKS		= false;
		bool	KNOWN_MAP_DRAW		= false;
		bool	KNOWN_MAP_CLEAR		= true;
		bool	FRAMERATE			= true;
		bool	GRID_OCCUPANCY		= false;
	} debugFlags;

public:

	bool											InitSystem();
	void											ShutdownSystem();
	void											Run();
	void											Stop();

	virtual bool									Init() = 0;
	virtual void									Shutdown() = 0;
	virtual void									Update() = 0;

	eAudio &										GetAudio();
	eInput &										GetInput();
	eRenderer &										GetRenderer();
	eImageManager &									GetImageManager();
	eAnimationManager &								GetAnimationManager();
	eAnimationControllerManager &					GetAnimationControllerManager();
	eEntityPrefabManager &							GetEntityPrefabManager();

	// frame-rate metrics
	void											SetFixedFPS(const Uint32 newFPS);
	Uint32											GetFixedFPS() const;
	Uint32											GetFixedTime() const;
	Uint32											GetDynamicFPS() const;
	Uint32											GetDeltaTime() const;
	Uint32											GetGameTime() const;
	void											DrawFPS();

	virtual int										GetClassType() const override				{ return CLASS_GAME; }
	virtual bool									IsClassType(int classType) const override	{ 
														if(classType == CLASS_GAME) 
															return true; 
														return eClass::IsClassType(classType); 
													}

private:

	eAudio											audio;
	eInput											input;
	eRenderer										renderer;
	eImageManager									imageManager;
	eAnimationManager								animationManager;
	eAnimationControllerManager						animationControllerManager;
	eEntityPrefabManager							entityPrefabManager;

	const Uint32									defaultFPS = 60;
	Uint32											fixedFPS;			// constant framerate
	Uint32											frameTime;			// constant framerate governing time interval (depends on FixedFPS)
	Uint32											deltaTime;			// actual time a frame takes to execute (to the nearest millisecond)
	Uint32											gameTime;			// time elapsed since execution began (updated at the end of each frame)
	bool											isRunning = false;	// determines whether the game shuts down or continues
};

extern eGame *	game;								// the rest of the engine will only reference this, while all local/derived aspects stay hidden

//****************
// eGame::Stop
// ends the main game loop
//****************
inline void eGame::Stop() {
	isRunning = false;
}

//****************
// eGame::GetAudio
//****************
inline eAudio & eGame::GetAudio() {
	return audio;
}

//****************
// eGame::GetInput
//****************
inline eInput & eGame::GetInput() {
	return input;
}

//****************
// eGame::GetRenderer
//****************
inline eRenderer & eGame::GetRenderer() {
	return renderer;
}

//*****************
// eGame::GetImageManager
//*****************
inline eImageManager & eGame::GetImageManager() {
	return imageManager;
}

//*****************
// eGame::GetAnimationManager
//*****************
inline eAnimationManager & eGame::GetAnimationManager() {
	return animationManager;
}

//*****************
// eGame::GetAnimationControllerManager
//*****************
inline eAnimationControllerManager & eGame::GetAnimationControllerManager() {
	return animationControllerManager;
}

//*****************
// eGame::GetEntityPrefabManager
//*****************
inline eEntityPrefabManager & eGame::GetEntityPrefabManager() {
	return entityPrefabManager;
}

//****************
// eGame::SetFixedFPS
//****************
inline void eGame::SetFixedFPS(const Uint32 newFPS) {
	fixedFPS = newFPS;
	frameTime = 1000 / fixedFPS;
}

//****************
// eGame::GetFixedFPS
//****************
inline Uint32 eGame::GetFixedFPS() const {
	return fixedFPS;
}

//****************
// eGame::GetFixedTime
//****************
inline Uint32 eGame::GetFixedTime() const {
	return frameTime;
}

//****************
// eGame::GetDynamicFPS
//****************
inline Uint32 eGame::GetDynamicFPS() const {
	if (deltaTime)
		return 1000 / deltaTime;
	else
		return fixedFPS;
}

//****************
// eGame::GetDeltaTime
//****************
inline Uint32 eGame::GetDeltaTime() const {
	return deltaTime;
}

//****************
// eGame::GetGameTime
//****************
inline Uint32 eGame::GetGameTime() const {
	return gameTime;
}

#endif /* EVIL_GAME_H */