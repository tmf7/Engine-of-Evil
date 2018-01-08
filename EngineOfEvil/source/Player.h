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
#ifndef EVIL_PLAYER_H
#define EVIL_PLAYER_H

#include "Entity.h"

namespace logic {

	using namespace evil;

//*************************************************
//					ePlayer
// handles all player input controls
// this is a higher level game-logic class for testing
// rather than a generic game-engine class
//*************************************************
class ePlayer : public eGameObject {

	ECLASS_DECLARATION(ePlayer)

public:

	virtual void							Think() override;
	virtual void							DebugDraw(eRenderTarget * renderTarget) override;
	void									Draw();

	// DEBUG: this type of player has static storage outside the eMap it uses, so no re-allocation is needed
	void									SetMap(eMap * newMap)	{ map = newMap; }

private:

	bool									SelectGroup();
	void									ClearGroupSelection();

private:

	std::vector<eEntity *>					groupSelection;
	std::array<eVec2, 2>					selectionPoints;			// on-screen selection box, and converts to worldspace for eEntity selection
	bool									beginSelection = false;
};

}	   /* logic */
#endif /* EVIL_PLAYER_H */
