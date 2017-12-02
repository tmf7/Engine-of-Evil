#ifndef ENTITY_H
#define ENTITY_H

#include "GameObject.h"
#include "Resource.h"

// entitySpawnArgs_t
typedef struct entitySpawnArgs_s {
	eBounds			localBounds;
	std::string		sourceFilename				= "";
	std::string		spriteFilename				= "";
	std::string		animationControllerFilename = "";
	eVec3			renderBlockSize				= vec3_zero;
	eVec2			imageColliderOffset			= vec2_zero;
	float			movementSpeed				= 0.0f;
	int				prefabManagerIndex			= 0;
	int				initialSpriteFrame			= 0;
	bool			collisionActive				= false;
//	int				colliderType;				// TODO: AABB/OBB/Circle/Line/Polyline
//	int				colliderMask;				// TODO: solid, liquid, enemy, player, etc
		
					entitySpawnArgs_s() { localBounds.Clear(); };
} entitySpawnArgs_t;

//*************************************************
//					eEntity
// objects that dynamically interact with the game environment
//*************************************************
class eEntity : public eGameObject, public eResource {
public:

								eEntity(const entitySpawnArgs_t & spawnArgs);

	static bool					Spawn(const int entityPrefabIndex, const eVec3 & worldPosition);

	void						SetPlayerSelected(bool isSelected);
	bool						GetPlayerSelected() const;
	void						UpdateRenderImageOrigin();
	void						UpdateRenderImageDisplay();
	int							SpawnID() const;

	virtual void				Think() override;			
	virtual void				DebugDraw() override;
	virtual int					GetClassType() const override { return CLASS_ENTITY; }

private:

	// DEBUG: collisionModel.Origin(), collisionModel.AbsBounds().Center and renderImage.origin 
	// are treated different in eEntity compared to eTile. 
	// --> eEntity moves via its collisionModel::origin so its localBounds is centered on (0,0)
	// and its renderImage.origin is positioned and offset (post-isometric-transformation) from the collisionModel.AbsBounds()[0] (minimum/top-left corner)
	// --> eTile does not move, and are positioned once according to its owner eGridCell position, then isometrically transformed
	// and its collisionModel.LocalBounds()[0] == renderImage.origin (pre-isometric-transformation) + eVec2(arbitraryOffset)
	// --> the reason this discrepancy currently remains is that eAI logic depends on a centered absBounds origin
	// TODO: update eAI logic to work from a universal eTransform::origin, and make
	// renderImage::origin, and collisionModel::origin positioned via offsets from that shared eTransform::origin
	// for both eTile and eEntity (static and dynamic objects)
	eVec2						imageColliderOffset;	
	int							spawnedEntityID;		// index within eGame::entities
	bool						playerSelected;			// player is controlling this eEntity

// FREEHILL BEGIN logic test
	eVec2 oldFacingDirection			= vec2_oneZero;
	const int xSpeedParameterHash		= std::hash<std::string>()("xSpeed");
	const int ySpeedParameterHash		= std::hash<std::string>()("ySpeed");
	const int magnitudeParameterHash	= std::hash<std::string>()("magnitude");
// FREEHILL END logic test
};

//**************
// eEntity::SetPlayerSelected
//**************
inline void eEntity::SetPlayerSelected(bool isSelected) {
	playerSelected = isSelected;
}

//**************
// eEntity::GetPlayerSelected
//**************
inline bool eEntity::GetPlayerSelected() const {
	return playerSelected;
}

//**************
// eEntity::SpawnID
//**************
inline int eEntity::SpawnID() const {
	return spawnedEntityID;
}

#endif /* ENTITY_H */