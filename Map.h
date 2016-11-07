#ifndef EVIL_MAP_H
#define EVIL_MAP_H

#include "Definitions.h"
#include "Math.h"
#include "SpatialIndexGrid.h"
#include "Bounds.h"
#include "Image.h"

class eGame;

class eMap {
public:

	// TODO: make camera a separate (entity?) class
	struct {
		eBounds localBounds;	// using local coordinates
		eBounds absBounds;		// using map coordinates
		eVec2 origin;			// center of bounds
		eVec2 velocity;
		float speed;
	} camera;

	enum tileType {
		TRAVERSABLE_TILE,
		COLLISION_TILE,
		RANDOM_TILE
	};

							eMap();

	bool					Init(char filename[], eGame * const game, int maxRows, int maxCols);
	bool					IsValid(const eVec2 & point, bool ignoreCollision = false);
	void					Update();
	void					BuildTiles(const int type);
	void					ToggleTile(const eVec2 & point);
	const game_map_t &		TileMap() const;

private:

	eGame *					game;
	eImage *				tileSet;
	game_map_t				tileMap;

	void					CameraInput();
	void					UpdateCameraOrigin();
	void					SetCameraOrigin(const eVec2 & point);
};

//**************
// eMap::eMap
//**************
inline eMap::eMap() {
}

//**************
// eMap::TileMap
//**************
inline const game_map_t & eMap::TileMap() const {
	return tileMap;
}

//**************
// eMap::UpdateCameraOrigin
// TODO: make this part of a camera class instead of a member struct of Map
// Adjust the user's view within map
//**************
inline void eMap::UpdateCameraOrigin() {
	camera.origin += camera.velocity * camera.speed;
	camera.absBounds = camera.localBounds + camera.origin;
}

//**************
// eMap::SetCameraOrigin
// TODO: make this part of a camera class instead of a member struct of Map
// Adjust the user's view within map
//**************
inline void eMap::SetCameraOrigin(const eVec2 & point) {
	camera.origin = point;
	camera.absBounds = camera.localBounds + camera.origin;
}

#endif /* EVIL_MAP_H */
