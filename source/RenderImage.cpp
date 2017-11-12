#include "Game.h"

//************
// eRenderImage::SetLayer
//************
void eRenderImage::SetRenderBlockZFromLayer(const int newLayer) {
	const float newRBMinZ = (float)game.GetMap().TileMap().MinZPositionFromLayer(newLayer);
	const float oldRBMinZ = renderBlock[0].z;
	renderBlock += eVec3(0.0f, 0.0f, newRBMinZ - oldRBMinZ);
	layer = newLayer;
}

//************
// eRenderImage::SetLayer
//************
void eRenderImage::UpdateLayerFromRenderBlockZ() {
	layer = game.GetMap().TileMap().LayerFromZPosition(eMath::NearestInt(renderBlock[0].z));
}

//************
// eRenderImage::AssignToWorldGrid
// assign drawing responsibility to eGridCells visually overlapped by worldClip's corners
// DEBUG(performance): ensures no renderImage suddenly dissappears when scrolling the camera
// for a single tileMap layer this results in each eGridCell::tileToDraw::size of:
// 4 : 6 : 8, for center : edge : corner on average
// more layers increases sizes (eg: 3 layers is about 4-6 : 11 : 20, depending on map design)
//************
void eRenderImage::AssignToWorldGrid() {
	std::array<eVec2, 4> visualWorldPoints;
	worldClip.ToPoints(visualWorldPoints.data());

	// clip rectangle to orthographic world-space for proper grid alignment
	auto & tileMap = game.GetMap().TileMap();
	for (auto & point : visualWorldPoints) {
		eMath::IsometricToCartesian(point.x, point.y);
		auto & cell = tileMap.IndexValidated(point);
		auto & searchTiles = cell.RenderContents();
		if (searchTiles.find(this) == searchTiles.end())	// don't add the same renderImage twice
			searchTiles[this] = this;
	}
}