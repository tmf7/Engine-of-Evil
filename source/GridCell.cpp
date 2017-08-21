#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() {
	for (auto && tile : tiles) {
		eVec2 drawPoint = tile.GetRenderImage()->origin - game.GetCamera().CollisionModel().AbsBounds()[0];
		drawPoint.SnapInt();
		tile.GetRenderImage()->dstRect = {	(int)drawPoint.x, 
											(int)drawPoint.y, 
											tile.GetRenderImage()->srcRect->w, 
											tile.GetRenderImage()->srcRect->h	};
		game.GetRenderer().AddToRenderPool(tile.GetRenderImage(), RENDERTYPE_DYNAMIC);
	}
}

