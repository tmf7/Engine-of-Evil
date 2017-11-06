#include "Player.h"
#include "Game.h"

void ePlayer::Think() {
	auto & input = game.GetInput();
	eVec2 selectionPoints[2];
	eVec2 mousePosition = eVec2((float)input.GetMouseX(), (float)input.GetMouseY());

	if (input.KeyPressed(SDL_SCANCODE_SPACE))
		ClearGroupSelection();

	if (input.MouseMoved())
		selectionPoints[1] = mousePosition;

	if (input.MousePressed(SDL_BUTTON_LEFT)) {
		selectionPoints[0] = mousePosition;
		beginSelection = groupSelection.empty();

	} else if (input.MouseReleased(SDL_BUTTON_LEFT)) {
		if (beginSelection) {
			beginSelection = false;
			SelectGroup(eBounds(selectionPoints, 2));

		} else if (!groupSelection.empty()) {
			for (auto & entity : groupSelection)
				entity->MovementPlanner().AddUserWaypoint(game.GetCamera().ScreenToWorldPosition(mousePosition));
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

	// draw the selection box
	// TODO: make this part of an ePlayer::Draw()
	if (beginSelection)
		game.GetRenderer().DrawCartesianRect(greenColor, eBounds(selectionPoints, 2), false, RENDERTYPE_DYNAMIC);
	else
		selectionPoints[0] = selectionPoints[1];

	// highlight the cell under the cursor
	// TODO: make this part of an ePlayer::Draw()
	eVec2 tilePoint = game.GetCamera().ScreenToWorldPosition(mousePosition);
	auto & tileMap = game.GetMap().TileMap();
	if (tileMap.IsValid(tilePoint)) {
		auto & tileBounds = tileMap.Index(tilePoint).AbsBounds();
		game.GetRenderer().DrawIsometricRect(yellowColor, tileBounds, RENDERTYPE_DYNAMIC);
	}


	auto & ownerCollisionModel = owner->collisionModel;			// FIXME: whoever is currently being controlled (potentially more than one eEntity)
	auto & ownerVelocity = ownerCollisionModel->Velocity();

	if (input.KeyPressed(SDL_SCANCODE_M)) {
		pathingState = (pathingState == PATHTYPE_COMPASS ? PATHTYPE_WALL : PATHTYPE_COMPASS);
		moveState = MOVETYPE_GOAL;
	} 

	if (game.debugFlags.KNOWN_MAP_CLEAR && input.KeyPressed(SDL_SCANCODE_R))
		ClearTrail();

	float xMove = maxMoveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_RIGHT) - input.KeyHeld(SDL_SCANCODE_LEFT));
	float yMove = maxMoveSpeed * (float)(input.KeyHeld(SDL_SCANCODE_DOWN) - input.KeyHeld(SDL_SCANCODE_UP));

	if (SDL_fabs(xMove) > 0.0f || SDL_fabs(yMove) > 0.0f) {
		eVec2 moveInput(xMove, yMove);
		moveInput.Normalize();
		moveInput *= maxMoveSpeed;
		eMath::IsometricToCartesian(moveInput.x, moveInput.y);
		ownerVelocity.Set(moveInput.x, moveInput.y);
		ownerCollisionModel->UpdateOrigin();
	}
}

//***************
// ePlayer::SelectGroup
// TODO: make sure the selectionArea param is in world space, not screen space
//***************
void ePlayer::SelectGroup(const eBounds & selectionArea) {
	static std::unordered_map<eEntity *, eEntity *> alreadyTested;
	static std::vector<eGridCell *> selectedCells;				// DEBUG(performance): static to reduce dynamic allocations	

	eCollision::GetAreaCells(selectionArea, selectedCells);
	for (auto & cell : selectedCells) {
		for (auto & kvPair : cell->Contents()) {
			auto & entity = kvPair.second->owner;				// FIXME(!): give eCollisionModel's proper owner's (eEntity or otherwise inherited)

			// don't test the same entity twice
			if (alreadyTested.find(entity) != alreadyTested.end())
				continue;

			alreadyTested[entity] = entity;
			if (eCollision::AABBAABBTest(entity->RenderImage()->worldClip, selectionArea)) {
				entity->selected = true;						// TODO: to draw a highlight around the selected entities
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
		entity->selected = false;
	}
	groupSelection.clear();
}