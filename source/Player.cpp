#include "Player.h"
#include "Game.h"

void ePlayer::Think() {
	auto & input = game.GetInput();
	eVec2 screenPosition = eVec2((float)input.GetMouseX(), (float)input.GetMouseY()); 

	if (input.KeyPressed(SDL_SCANCODE_SPACE))
		ClearGroupSelection();

	if (input.MouseMoved())
		selectionPoints[1] = screenPosition;

	if (input.MousePressed(SDL_BUTTON_LEFT)) {
		selectionPoints[0] = screenPosition;
		beginSelection = groupSelection.empty();

	} else if (input.MouseReleased(SDL_BUTTON_LEFT)) {
		if (beginSelection) {
			beginSelection = false;
			SelectGroup();

		} else if (!groupSelection.empty()) {
			for (auto & entity : groupSelection)
				entity->MovementPlanner().AddUserWaypoint(game.GetCamera().MouseWorldPosition());
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

		if (game.debugFlags.KNOWN_MAP_CLEAR && input.KeyPressed(SDL_SCANCODE_R))
			entityMovement.ClearTrail();

		const float moveSpeed = entityMovement.Speed();
		const float xMove = moveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_RIGHT) - input.KeyHeld(SDL_SCANCODE_LEFT));
		const float yMove = moveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_DOWN) - input.KeyHeld(SDL_SCANCODE_UP));

		if (SDL_fabs(xMove) > 0.0f || SDL_fabs(yMove) > 0.0f) {
			eVec2 moveInput(xMove, yMove);
			moveInput.Normalize();
			moveInput *= moveSpeed;
			eMath::IsometricToCartesian(moveInput.x, moveInput.y);
			auto & entityCollisionModel = entity->CollisionModel();
			entityCollisionModel.Velocity().Set(moveInput.x, moveInput.y);
			entityCollisionModel.UpdateOrigin();
		}
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
	static std::vector<eGridCell *> selectedCells;				// DEBUG(performance): static to reduce dynamic allocations	

	eBounds selectionBounds(selectionPoints.data(), selectionPoints.size());
	if (selectionBounds.Width() <= 0.0f || selectionBounds.Height() <= 0.0f)
		return false;

	// similar to eRenderImage::UpdateAreasWorldClipArea (except for camera adjustment)
	eVec2 corner = selectionBounds[0];
	eVec2 xAxis(selectionBounds[1].x, selectionBounds[0].y);
	eVec2 yAxis(selectionBounds[0].x, selectionBounds[1].y);
	std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
	for (auto & point : obbPoints) {
		point += game.GetCamera().CollisionModel().AbsBounds()[0];
		eMath::IsometricToCartesian(point.x, point.y);
	}
	eBox selectionArea(obbPoints.data());

	eCollision::GetAreaCells(selectionArea, selectedCells);
	for (auto & cell : selectedCells) {
		for (auto & kvPair : cell->RenderContents()) {
			auto owner = kvPair.second->Owner();
			
			if (owner == nullptr || owner->GetClassType() != CLASS_ENTITY)
				continue;

			const auto & entity = static_cast<eEntity *>(owner);
			
			// don't test the same entity twice
			if (alreadyTested.find(entity) != alreadyTested.end())
				continue;

			alreadyTested[entity] = entity;
			const eBounds dstRect = entity->RenderImage().GetWorldClip().Translate(-game.GetCamera().CollisionModel().AbsBounds()[0]);
			if (eCollision::AABBAABBTest(dstRect, selectionBounds)) {
				entity->SetPlayerSelected(true);						// TODO: to draw a highlight around the selected entities
				groupSelection.emplace_back(entity);
			}
		}
	}
	alreadyTested.clear();
	selectedCells.clear();
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
		game.GetRenderer().DrawCartesianRect(greenColor, eBounds(selectionPoints.data(), selectionPoints.size()) , false, RENDERTYPE_STATIC);
	else
		selectionPoints[0] = selectionPoints[1];

	// highlight those selected
	for (auto & entity : groupSelection)
		game.GetRenderer().DrawIsometricPrism(lightBlueColor, entity->RenderImage().RenderBlock(), RENDERTYPE_DYNAMIC);

}

//***************
// ePlayer::DebugDraw
//***************
void ePlayer::DebugDraw() {
	// highlight the cell under the cursor
	// FIXME: conflicts w/ ePlayerDraw order (because of the immediacy of drawing to the rendertargets)
	auto & tileMap = game.GetMap().TileMap();
	const eVec2 worldPosition = game.GetCamera().MouseWorldPosition();
	if (tileMap.IsValid(worldPosition)) {
		auto & tileBounds = tileMap.Index(worldPosition).AbsBounds();
//		game.GetRenderer().DrawIsometricRect(yellowColor, tileBounds, RENDERTYPE_DYNAMIC);
	}
}