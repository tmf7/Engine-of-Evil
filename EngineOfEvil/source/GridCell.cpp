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
#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() {
	auto & renderer = game.GetRenderer();
	for (auto & contentPair : renderContents)
		renderer.AddToCameraRenderPool(contentPair.second);
}

//************
// eGridCell::AddTileOwned
//************
void eGridCell::AddTileOwned(eTile && tile) {
	tilesOwned.emplace_back(std::move(tile));
}

//************
// eGridCell::Reset
//************
void eGridCell::Reset() {
	eGridIndex::Reset();
	collisionContents.clear();
	renderContents.clear();
	tilesOwned.clear();
//	absBounds.Clear();			// if loading new data into the same eSpatialIndexGrid<eGridCell...>, then absBounds should be re-initialized regardless
}

//************
// eGridCell::DebugDraw
//************
void eGridCell::DebugDraw() {
	auto & renderer = game.GetRenderer();
	for (auto & tile : tilesOwned) {
		if (game.debugFlags.COLLISION && &tile.CollisionModel() != nullptr)
			game.GetRenderer().DrawIsometricRect(pinkColor, tile.CollisionModel().AbsBounds(), true);

		auto & renderBlock = tile.RenderImage().GetRenderBlock();
		if (game.debugFlags.RENDERBLOCKS && renderBlock.Depth() > 0)		// DEBUG(performance): for visual clarity, don't draw flat renderBlocks
			game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderBlock, RENDERTYPE_DYNAMIC);
	}
}
