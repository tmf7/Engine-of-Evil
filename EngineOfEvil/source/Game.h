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
#include "Collision.h"
#include "Renderer.h"

namespace evil {

//*************************************************
//					eGame
// handles all sub-system loading, initialization, and unloading
//*************************************************
class eGame : public eClass {

	ECLASS_DECLARATION(eGame)

public:

	bool											InitSystem();
	void											ShutdownSystem();
	void											Run();

													// ends the main game loop
	void											Stop()								{ isRunning = false; }

	virtual bool									Init() = 0;
	virtual void									Shutdown() = 0;
	virtual void									Update() = 0;

	eAudio &										GetAudio()							{ return audio; }
	eInput &										GetInput()							{ return input; }
	eRenderer &										GetRenderer()						{ return renderer; }
	eImageManager &									GetImageManager()					{ return imageManager; }
	eAnimationManager &								GetAnimationManager()				{ return animationManager; }
	eAnimationControllerManager &					GetAnimationControllerManager()		{ return animationControllerManager; }
	eEntityPrefabManager &							GetEntityPrefabManager()			{ return entityPrefabManager; }

	// frame-rate metrics
	void											SetFixedFPS(const Uint32 newFPS);
	Uint32											GetDynamicFPS() const;
	Uint32											GetFixedFPS() const					{ return fixedFPS; }
	float											GetFixedTime() const				{ return frameTime; }
	float											GetDeltaTime() const				{ return deltaTime; }
	float											GetGameTime() const					{ return gameTime; }
	void											DrawFPS();

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
	float											frameTime;			// constant framerate time interval in seconds (depends on FixedFPS)
	float											deltaTime;			// actual time a frame takes to execute in seconds (to the nearest millisecond)
	float											gameTime;			// time elapsed since execution began in seconds (updated at the end of each frame)
	bool											isRunning = false;	// determines whether the game shuts down or continues
};

extern eGame *	game;								// the rest of the engine will only reference this, while all local/derived aspects stay hidden

//****************
// eGame::SetFixedFPS
//****************
inline void eGame::SetFixedFPS(const Uint32 newFPS) {
	fixedFPS = newFPS;
	frameTime = 1.0f / (float)fixedFPS;
}

//****************
// eGame::GetDynamicFPS
//****************
inline Uint32 eGame::GetDynamicFPS() const {
	if (deltaTime)
		return (Uint32)eMath::NearestInt(1.0f / deltaTime);
	else
		return fixedFPS;
}

}	   /* evil */
#endif /* EVIL_GAME_H */