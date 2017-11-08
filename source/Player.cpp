#include "Player.h"
#include "Game.h"

void ePlayer::Think() {
	auto & input = game.GetInput();
	const eVec2 worldPosition = game.GetCamera().MouseWorldPosition();

	if (input.KeyPressed(SDL_SCANCODE_SPACE))
		ClearGroupSelection();

	if (input.MouseMoved())
		selectionPoints[1] = worldPosition;

	if (input.MousePressed(SDL_BUTTON_LEFT)) {
		selectionPoints[0] = worldPosition;
		beginSelection = groupSelection.empty();

	} else if (input.MouseReleased(SDL_BUTTON_LEFT)) {
		if (beginSelection) {
			beginSelection = false;
			SelectGroup(eBounds(selectionPoints.data(), selectionPoints.size()));

		} else if (!groupSelection.empty()) {
			for (auto & entity : groupSelection)
				entity->MovementPlanner().AddUserWaypoint(worldPosition);
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
// DEBUG: selectionArea must be in world space, not screen space
//***************
void ePlayer::SelectGroup(const eBounds & selectionArea) {
	static std::unordered_map<const eEntity *, const eEntity *> alreadyTested;
	static std::vector<eGridCell *> selectedCells;				// DEBUG(performance): static to reduce dynamic allocations	

	eCollision::GetAreaCells(selectionArea, selectedCells);
	for (auto & cell : selectedCells) {
		for (auto & kvPair : cell->Contents()) {
			auto & ownerClass = kvPair.second->Owner();		// FIXME/BUG(!): grabbing cells read-access violation (&owner may have moved)
			
			if (ownerClass.GetClassType() != CLASS_ENTITY)
				continue;

			const auto & entity = static_cast<eEntity *>(&ownerClass);
			
			// don't test the same entity twice
			if (alreadyTested.find(entity) != alreadyTested.end())
				continue;

			alreadyTested[entity] = entity;
			if (eCollision::AABBAABBTest(entity->GetRenderImage()->worldClip, selectionArea)) {
				entity->PlayerSelected(true);						// TODO: to draw a highlight around the selected entities
				groupSelection.push_back(entity);
			}
		}
	}
	alreadyTested.clear();
	selectedCells.clear();

	// TODO: test if selectionArea is below a certain size threshold
	// that indicates the intention was to select a single eEntity
	// then loop over groupSelection looking for the selected item closest to the camera (ie: localDepthSort)
	// then grab it, ClearGroupSelection(), and re-add the one eEntity
}

//***************
// ePlayer::ClearGroupSelection
//***************
void ePlayer::ClearGroupSelection() {
	for (auto & entity : groupSelection) {
		entity->PlayerSelected(false);
	}
	groupSelection.clear();
}


//***************
// ePlayer::Draw
//***************
void ePlayer::Draw() {
	// draw the selection box
	if (beginSelection)
		game.GetRenderer().DrawIsometricRect(greenColor, eBounds(selectionPoints.data(), selectionPoints.size()), RENDERTYPE_DYNAMIC);
	else
		selectionPoints[0] = selectionPoints[1];

	// highlight those selected
	for (auto & entity : groupSelection) {
		game.GetRenderer().DrawIsometricPrism(lightBlueColor, entity->GetRenderImage()->renderBlock, RENDERTYPE_DYNAMIC);
	}
}

//***************
// ePlayer::DebugDraw
//***************
void ePlayer::DebugDraw() {
	// highlight the cell under the cursor
	auto & tileMap = game.GetMap().TileMap();
	const eVec2 worldPosition = game.GetCamera().MouseWorldPosition();
	if (tileMap.IsValid(worldPosition)) {
		auto & tileBounds = tileMap.Index(worldPosition).AbsBounds();
		game.GetRenderer().DrawIsometricRect(yellowColor, tileBounds, RENDERTYPE_DYNAMIC);
	}
}