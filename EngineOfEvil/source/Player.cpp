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
#include "GameLocal.h"
#include "Player.h"
#include "sHero.h"

void ePlayer::Think() {
	auto & input = game->GetInput();
	eVec2 screenPosition = eVec2((float)input.GetMouseX(), (float)input.GetMouseY()); 

	if (input.KeyPressed(SDL_SCANCODE_SPACE))
		ClearGroupSelection();

	if (input.MouseMoved())
		selectionPoints[1] = screenPosition;

	if (input.MousePressed(static_cast<SDL_Scancode>(SDL_BUTTON_LEFT))) {
		selectionPoints[0] = screenPosition;
		beginSelection = groupSelection.empty();

	} else if (input.MouseReleased(static_cast<SDL_Scancode>(SDL_BUTTON_LEFT))) {
		if (beginSelection) {
			beginSelection = false;
			SelectGroup();

		} else if (!groupSelection.empty()) {
			for (auto & entity : groupSelection)
				entity->MovementPlanner().AddUserWaypoint(map->GetViewCamera()->MouseWorldPosition());
/*
			if ("small selection area, so set a chase target for the group if there's an eEntity in the selectionArea") {		// TODO: implement
			} else { // group pathfinding
				for (auto & entity : groupSelection) {
					entity->goal = &cell.center;
					PathFind(entity, SDL_Point{ (int)entity->center.x, (int)entity->center.y }, second);
				}
			}
*/
		}
	}

	for (auto & entity : groupSelection) {	
		auto & entityMovement = entity->MovementPlanner();
		if (input.KeyPressed(SDL_SCANCODE_M))
			entityMovement.TogglePathingState();

		if (game->debugFlags.KNOWN_MAP_CLEAR && input.KeyPressed(SDL_SCANCODE_R))
			entityMovement.ClearTrail();

		const float moveSpeed = entityMovement.Speed();
		const float xMove = moveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_RIGHT) - input.KeyHeld(SDL_SCANCODE_LEFT));
		const float yMove = moveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_DOWN) - input.KeyHeld(SDL_SCANCODE_UP));

		eVec2 moveInput(xMove, yMove);
		moveInput.Normalize();
		moveInput *= moveSpeed;
		eMath::IsometricToCartesian(moveInput.x, moveInput.y);
		auto & entityCollisionModel = entity->CollisionModel();
		entityCollisionModel.SetVelocity(eVec2(moveInput.x, moveInput.y));
	}
}

//***************
// ePlayer::SelectGroup
// converts selectionPoints to a worldspace bounding box 
// used to select eEntities
// returns true of !groupSelection.empty()
// returns false if selection area has zero area, or groupSelection.empty()
// TODO: narrow-phase per-pixel collision test between selection area and worldClips collided with
// to only select eEntities based on opaque pixels (and allow more precise single-eEntity selection)
//***************
bool ePlayer::SelectGroup() {
	static std::unordered_map<const eEntity *, const eEntity *> alreadyTested;
	static std::vector<eGridCell *>								selectedCells;				// DEBUG(performance): static to reduce dynamic allocations	
	alreadyTested.clear();																	// lazy clearing
	selectedCells.clear();

	eBounds selectionBounds(selectionPoints.data(), selectionPoints.size());
	if (selectionBounds.Width() <= 0.0f || selectionBounds.Height() <= 0.0f)
		return false;

	// similar to eRenderImage::UpdateAreasWorldClipArea (except for camera adjustment)
	eVec2 corner = selectionBounds[0];
	eVec2 xAxis(selectionBounds[1].x, selectionBounds[0].y);
	eVec2 yAxis(selectionBounds[0].x, selectionBounds[1].y);
	std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
	for (auto & point : obbPoints)
		point = map->GetViewCamera()->ScreenToWorldPosition(point);

	eBox selectionArea(obbPoints.data());

	eCollision::GetAreaCells(map, selectionArea, selectedCells);
	for (auto & cell : selectedCells) {
		for (auto & kvPair : cell->RenderContents()) {
			auto owner = kvPair.second->Owner();
			
			if (owner == nullptr || !owner->IsClassType(CLASS_ENTITY))
				continue;

			const int classType = owner->GetClassType();
			const auto & entity = (classType == CLASS_ENTITY ? static_cast<eEntity *>(owner) : static_cast<sHero *>(owner));
			
			// don't test the same entity twice
			if (alreadyTested.find(entity) != alreadyTested.end())
				continue;

			alreadyTested[entity] = entity;

			// account for current camera zoom level
			auto & worldClip = entity->RenderImage().GetWorldClip();
			const float zoom = map->GetViewCamera()->GetZoom();
			const eVec2 worldClipOrigin = (worldClip[0] - map->GetViewCamera()->AbsBounds()[0]) * zoom;
			const eVec2 worldClipSize = eVec2(worldClip.Width(), worldClip.Height()) * zoom;
			const eBounds dstRect = eBounds(worldClipOrigin, worldClipOrigin + worldClipSize);
			
			if (eCollision::AABBAABBTest(dstRect, selectionBounds)) {
				entity->SetPlayerSelected(true);
				groupSelection.emplace_back(entity);
			}
		}
	}
	return !groupSelection.empty();
}

//***************
// ePlayer::ClearGroupSelection
//***************
void ePlayer::ClearGroupSelection() {
	for (auto & entity : groupSelection) {
		entity->SetPlayerSelected(false);
	}
	groupSelection.clear();
}

//***************
// ePlayer::Draw
//***************
void ePlayer::Draw() {
	// draw the selection box
	if (beginSelection)
		game->GetRenderer().DrawCartesianRect(game->GetRenderer().GetDebugOverlayTarget(), greenColor, eBounds(selectionPoints.data(), selectionPoints.size()) , false);
	else
		selectionPoints[0] = selectionPoints[1];

	// highlight those selected
	for (auto & entity : groupSelection)
		game->GetRenderer().DrawIsometricRect(map->GetViewCamera()->GetDebugRenderTarget(), lightBlueColor, entity->CollisionModel().AbsBounds());
}

//***************
// ePlayer::DebugDraw
//***************
void ePlayer::DebugDraw(eRenderTarget * renderTarget) {
	// highlight the cell under the cursor
	auto & tileMap = map->TileMap();
	const eVec2 worldPosition = map->GetViewCamera()->MouseWorldPosition();
	if (tileMap.IsValid(worldPosition)) {
		auto & tileBounds = tileMap.Index(worldPosition).AbsBounds();
		game->GetRenderer().DrawIsometricRect(renderTarget,yellowColor, tileBounds);
	}

}