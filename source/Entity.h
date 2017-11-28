#ifndef ENTITY_H
#define ENTITY_H

#include "GameObject.h"

// entitySpawnArgs_t
typedef struct entitySpawnArgs_s {
	eBounds			localBounds;
	std::string		sourceFilename		= "";
	std::string		spriteFilename		= "";
	eVec3			renderBlockSize		= vec3_zero;
	eVec2			imageColliderOffset = vec2_zero;
	float			movementSpeed		= 0.0f;
	int				prefabManagerIndex	= 0;
	bool			collisionActive		= false;
//	int				colliderType;		// TODO: AABB/OBB/Circle/Line/Polyline
//	int				colliderMask;		// TODO: solid, liquid, enemy, player, etc
		
					entitySpawnArgs_s() { localBounds.Clear(); };
} entitySpawnArgs_t;

//*************************************************
//					eEntity
// objects that dynamically interact with the game environment
//*************************************************
class eEntity : public eGameObject {
public:

								eEntity(const entitySpawnArgs_t & spawnArgs);

	static bool					Spawn(const int entityPrefabIndex, const eVec3 & worldPosition/*, const eVec2 & facingDir*/);

	void						SetPlayerSelected(bool isSelected);
	bool						GetPlayerSelected() const;
	void						UpdateRenderImageOrigin();
	void						UpdateRenderImageDisplay();
	const std::string &			GetSourceFilename() const;
	int							PrefabManagerIndex() const;
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
	eVec2								imageColliderOffset;	

	std::string							sourceFilename;
	int									prefabManagerIndex;		// index of this eEntity's prefab within eEntityPrefabManager::prefabList
	int									spawnedEntityID;
	bool								playerSelected;			// player is controlling this eEntity
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
// eEntity::GetPrefabFilename
//**************
inline const std::string & eEntity::GetSourceFilename() const {
	return sourceFilename;
}

//**************
// eEntity::PrefabManagerIndex
//**************
inline int eEntity::PrefabManagerIndex() const {
	return prefabManagerIndex;
}

//**************
// eEntity::SpawnID
//**************
inline int eEntity::SpawnID() const {
	return spawnedEntityID;
}

#endif /* ENTITY_H */