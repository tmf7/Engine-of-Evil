#ifndef ENTITY_H
#define ENTITY_H

#include "GameObject.h"
#include "Resource.h"
#include "Dictionary.h"

// entitySpawnArgs_t
typedef struct entitySpawnArgs_s {
	eBounds			localBounds;
	std::string		sourceFilename				= "";
	std::string		spriteFilename				= "";
	std::string		animationControllerFilename = "";
	eVec3			renderBlockSize				= vec3_zero;
	eVec2			renderImageOffset			= vec2_zero;
	eVec2			colliderOffset				= vec2_zero;
	float			movementSpeed				= 0.0f;
	int				prefabManagerIndex			= 0;
	int				initialSpriteFrame			= 0;
	bool			collisionActive				= false;
//	int				colliderType;				// TODO: AABB/OBB/Circle/Line/Polyline
//	int				colliderMask;				// TODO: solid, liquid, enemy, player, etc
//  Uint32			worldLayer;					// TODO: use this instead of Spawn(worldPosition.z)
		
					entitySpawnArgs_s() { localBounds.Clear(); };
} entitySpawnArgs_t;

//*************************************************
//					eEntity
// objects that dynamically interact with the game environment
//*************************************************
class eEntity : public eGameObject, public eResource {
public:

	friend class eGame;			// for access to assign spawnedEntityID

public:

//	virtual					   ~eEntity() override = default;
//								eEntity() = default;
//								eEntity(const eEntity & other) = default;
//								eEntity(eEntity && other) = default;
								eEntity(const char * sourceFilename, int managerIndex);

//	eEntity &					operator=(const eEntity & other)				{ eGameObject::operator=(other); }
//	eEntity &					operator=(eEntity && other)						{ eGameObject::operator=(other); }

	void						SetPlayerSelected(bool isSelected);
	bool						GetPlayerSelected() const;
	void						SetSpawnArgs(const eDictionary & newSpawnArgs);
	const eDictionary &			GetSpawnArgs() const;
	int							SpawnID() const;

	virtual bool				Spawn(const eVec3 & worldPosition);
	virtual void				DebugDraw() override;

	virtual int					GetClassType() const override					{ return CLASS_ENTITY; }
	virtual bool				IsClassType(int classType) const override		{ 
									if(classType == CLASS_ENTITY) 
										return true; 
									return eGameObject::IsClassType(classType);
								}

protected:

	eDictionary					spawnArgs;				// populated during eEntityPrefabManager::CreatePrefab, used for initialization
	int							spawnedEntityID;		// index within eGame::entities
	bool						playerSelected;			// player is controlling this eEntity
};

//**************
// eEntity::eEntity
//**************
inline eEntity::eEntity( const char * sourceFilename, int managerIndex )
	: eResource( sourceFilename, managerIndex ) {
}

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
// eEntity::SetSpawnArgs
// DEBUG: best just to set this once a load-time
//**************
inline void eEntity::SetSpawnArgs(const eDictionary & newSpawnArgs) {
	spawnArgs = newSpawnArgs;
}

//**************
// eEntity::GetSpawnArgs
//**************
inline const eDictionary & eEntity::GetSpawnArgs() const {
	return spawnArgs;
}

//**************
// eEntity::SpawnID
//**************
inline int eEntity::SpawnID() const {
	return spawnedEntityID;
}

#endif /* ENTITY_H */