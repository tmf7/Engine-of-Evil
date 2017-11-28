#include "Game.h"

//*************
// eRenderImage::~eRenderImage
//*************
eRenderImage::~eRenderImage() {
	ClearAreas();
}

//************
// eRenderImage::SnapRenderBlockToLayer
// DEBUG: only call this after owner->worldLayer has been assigned
//************
void eRenderImage::SnapRenderBlockToLayer() {
	const float newRBMinZ = (float)game.GetMap().TileMap().MinZPositionFromLayer(owner->WorldLayer());
	const float oldRBMinZ = renderBlock[0].z;
	renderBlock += eVec3(0.0f, 0.0f, newRBMinZ - oldRBMinZ);
}

//************
// eRenderImage::ClearAreas
// removes this from all eMap::tileMap gridcells with pointers to it
// and clear this->areas gridcell pointers
//************
void eRenderImage::ClearAreas() {
	for (auto && cell : areas) {
		auto & contents = cell->RenderContents();
		if (contents.empty())	// FIXME: necessary to prevent a shutdown crash using std::unordered_map::find (insead of std::find) because of index reference
			continue;

		auto & index = contents.find(this);
		if (index != contents.end())
			contents.erase(index);
	}
	areas.clear();
}

//***************
// eRenderImage::UpdateAreasWorldClipCorners
// adds this to the eMap::tileMap gricells that contain the four corners of this->worldClip
// and adds those same gridcell pointers to this->areas
// DEBUG(performance): ensures no renderImage suddenly dissappears when scrolling the camera
// for a single tileMap layer this results in each eGridCell::renderContents::size of:
// 4 : 6 : 8, for center : edge : corner (parts of the tileMap) on average
// more layers increases sizes (eg: 3 layers is about 4-6 : 11 : 20, depending on map design)
//***************
void eRenderImage::UpdateAreasWorldClipCorners() {
	ClearAreas();

	std::array<eVec2, 4> visualWorldPoints;
	worldClip.ToPoints(visualWorldPoints.data());

	// clip rectangle to orthographic world-space for proper grid alignment
	auto & tileMap = game.GetMap().TileMap();
	for (auto & point : visualWorldPoints) {
		eMath::IsometricToCartesian(point.x, point.y);
		auto & cell = tileMap.IndexValidated(point);
		auto & searchContents = cell.RenderContents();
		if (searchContents.find(this) == searchContents.end()) {	// don't add the same renderImage or cell twice
			searchContents[this] = this;
			areas.emplace_back(&cell);
		}
	}
}

//***************
// eRenderImage::UpdateAreasWorldClipArea
// adds this to the eMap::tileMap gricells that this->worldClip overlaps
// and adds those same gridcell pointers to this->areas
// DEBUG(performance): only this to more eGridCells as needed (see: eRenderImage::UpdateAreasWorldClipCorners),
// also this fn relies on the expensive eCollision::OBBOBBTest instead of essentially-no collision test (like eRenderImage::UpdateAreasWorldClipCorners)
//***************
void eRenderImage::UpdateAreasWorldClipArea() {
	ClearAreas();

	eVec2 corner = worldClip[0];
	eVec2 xAxis(worldClip[1].x, worldClip[0].y);
	eVec2 yAxis(worldClip[0].x, worldClip[1].y);
	std::array<eVec2, 3> obbPoints = { std::move(corner), std::move(xAxis), std::move(yAxis) };
	for (auto & point : obbPoints)
		eMath::IsometricToCartesian(point.x, point.y);

	const eBox worldClipArea(obbPoints.data());
	eCollision::GetAreaCells(worldClipArea, areas);
	for (auto && cell : areas)
		cell->RenderContents()[this] = this;
}

//*************
// eRenderImage::SetOrigin
// DEBUG: if owner->IsStatic only call this during loadtime initialization, not each frame
//*************
void eRenderImage::SetOrigin(const eVec2 & newOrigin) {
	oldOrigin = origin;
	origin = newOrigin;
	UpdateWorldClip();
	if (owner->IsStatic() || origin != oldOrigin) {
		if (isSelectable)
			UpdateAreasWorldClipArea();
		else
			UpdateAreasWorldClipCorners();
	}
}