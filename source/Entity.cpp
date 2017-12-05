#include "Game.h"

//***************
// eEntity::Spawn
// copies a prefab eEntity and adds unique details
//***************
bool eEntity::Spawn(const eVec3 & worldPosition) {
	auto & newEntity = std::make_unique<eEntity>(*this);
	newEntity->SetZPosition(worldPosition.z);
	newEntity->SetOrigin(eVec2(worldPosition.x, worldPosition.y));
	return (game.AddEntity(std::move(newEntity)) >= 0);
}

//***************
// eEntity::DebugDraw
//***************
void eEntity::DebugDraw() {
	if (game.debugFlags.RENDERBLOCKS && animationController != nullptr)
		game.GetRenderer().DrawIsometricPrism(lightBlueColor, renderImage->GetRenderBlock(), RENDERTYPE_DYNAMIC);

//		game.GetRenderer().DrawCartesianRect(lightBlueColor, renderImage->GetWorldClip(), false, RENDERTYPE_DYNAMIC);		// DEBUG: draw worldclip

	// TODO: better visual difference b/t cells occupied by both renderImage and collisionmodel
	if (game.debugFlags.GRID_OCCUPANCY) {
		if (renderImage != nullptr) {
			for (auto & cell : renderImage->Areas()) {
				game.GetRenderer().DrawIsometricRect(yellowColor, cell->AbsBounds(), RENDERTYPE_DYNAMIC);
			}
		}

		if (collisionModel != nullptr) {
			for (auto & cell : collisionModel->Areas()) {
				game.GetRenderer().DrawIsometricRect(lightBlueColor, cell->AbsBounds(), RENDERTYPE_DYNAMIC);
			}
		}
	}

	if (game.debugFlags.COLLISION && collisionModel != nullptr)
		game.GetRenderer().DrawIsometricRect(yellowColor, collisionModel->AbsBounds(), RENDERTYPE_DYNAMIC);

	if (movementPlanner != nullptr)
		movementPlanner->DebugDraw();
}