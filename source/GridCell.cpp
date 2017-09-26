#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() {
	for (auto && tile : tilesToDraw) {
		game.GetRenderer().AddToRenderPool(tile->GetRenderImage(), RENDERTYPE_DYNAMIC);
	}
}

