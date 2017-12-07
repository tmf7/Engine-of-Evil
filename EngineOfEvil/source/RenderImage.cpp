#include "Game.h"

//*************
// eRenderImage::~eRenderImage
//*************
eRenderImage::~eRenderImage() {
	ClearAreas();
}

//*************
// eRenderImage::SetImage
// DEBUG: no range checking for faster assignment
//*************
void eRenderImage::SetImage(int imageManagerIndex) {
	image = game.GetImageManager().Get(imageManagerIndex);
}

//************
// eRenderImage::UpdateRenderBlock
// snaps the renderblock minimum z value according to owner::worldlayer, and
// the x,y is positioned using either owner::collisionModel (if non-nullptr), or owner::orthoOrigin,
// to best prevent renderBlock overlap and minimize draw-order sorting anomolies, otherwise the  is used
//************
void eRenderImage::UpdateRenderBlock() {
	auto & renderBlockMins = renderBlock[0];

	// static eGameObjects snap z according to worldLayer
	// dynamic eGameObjects use the fluid zPosition
	if (owner->worldLayer != owner->oldWorldLayer) {
		if (owner->isStatic) {
			const float newRBMinZ = (float)game.GetMap().TileMap().MinZPositionFromLayer(owner->worldLayer);
			renderBlock += eVec3(0.0f, 0.0f, newRBMinZ - renderBlockMins.z);
		} else {
			renderBlock += eVec3(0.0f, 0.0f, owner->zPosition - renderBlockMins.z);
		}
	}

	if (owner->collisionModel != nullptr) {
		auto & collisionMins = owner->collisionModel->AbsBounds()[0];
		renderBlock += eVec3(collisionMins.x - renderBlockMins.x, collisionMins.y - renderBlockMins.y , 0.0f);
	} else {
		auto & ownerOrigin = owner->GetOrigin();
		renderBlock += eVec3(ownerOrigin.x - renderBlockMins.x, ownerOrigin.y - renderBlockMins.y , 0.0f);
	}
}

//*************
// eRenderImage::SetRenderBlockSize
//*************
void eRenderImage::SetRenderBlockSize(const eVec3 & newSize) {
	auto & renderBlockMins = renderBlock[0];
	renderBlock = eBounds3D(renderBlockMins, renderBlockMins + newSize);
}

//************
// eRenderImage::Update
// ensures this->origin tracks along with owner::orthoOrigin
// ensures only the visuals are isometric, 
// while backend collision occurs on a 2D top-down grid
// DEBUG: if owner->IsStatic this only calls UpdateAreas during loadtime initialization, not each frame
//************
void eRenderImage::Update() {
	oldOrigin = origin;
	auto & ownerOrigin = owner->GetOrigin();
	eVec2 newOrigin = ownerOrigin;	
	eMath::CartesianToIsometric(newOrigin.x, newOrigin.y);
	newOrigin += orthoOriginOffset;
	origin = newOrigin;

	UpdateWorldClip();
	if (origin != oldOrigin || (owner->IsStatic() && game.GetGameTime() < 5000)) {
		if (isSelectable)
			UpdateAreasWorldClipArea();
		else
			UpdateAreasWorldClipCorners();
	}

	UpdateRenderBlock();
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
//*************
void eRenderImage::SetOrigin(const eVec2 & newOrigin) {
	owner->SetOrigin(newOrigin);
}