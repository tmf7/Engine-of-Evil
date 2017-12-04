#ifndef EVIL_SHERO_H
#define EVIL_SHERO_H

#include "Entity.h"

class sHero : public eEntity {
public:

										sHero(const eEntity & other) : eEntity(other)		{}
										sHero(eEntity && other) : eEntity(std::move(other)) {}

	virtual bool						Spawn(const eVec3 & worldPosition) override;
	virtual void						Think() override;
	virtual int							GetClassType() const override						{ return CLASS_SHERO; }
	virtual bool						IsClassType(int classType) const override			{ 
											if(classType == CLASS_SHERO) 
												return true; 
											return eEntity::IsClassType(classType);
										}

private:
	
	eVec2 oldFacingDirection			= vec2_oneZero;
	const int xSpeedParameterHash		= std::hash<std::string>()("xSpeed");
	const int ySpeedParameterHash		= std::hash<std::string>()("ySpeed");
	const int magnitudeParameterHash	= std::hash<std::string>()("magnitude");
};

#endif /* EVIL_SHERO_H */