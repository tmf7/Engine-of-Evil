#include "GridCell.h"
#include "Game.h"

//************
// eGridCell::Draw
//************
void eGridCell::Draw() const {
	for (auto && tile : tiles) {
		eVec2 screenPoint = eVec2(
			eMath::NearestFloat(tile.GetDrawOrigin().x - game.GetCamera().GetAbsBounds().x),
			eMath::NearestFloat(tile.GetDrawOrigin().y - game.GetCamera().GetAbsBounds().y)
		);
		const SDL_Rect & srcRect = tile.ImageFrame();
		SDL_Rect dstRect = SDL_Rect{ (int)screenPoint.x, (int)screenPoint.y, srcRect.w, srcRect.h };
		game.GetRenderer().AddToRenderPool(renderImage_t{ tileSet->Source(), &srcRect, dstRect, tile.GetLayer() }, RENDERTYPE_DYNAMIC);
	}
}
