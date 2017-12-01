#ifndef EVIL_SHERO_H
#define EVIL_SHERO_H

#include "Entity.h"

class sHero : public eEntity {
public:

	virtual void						ExtendedThink() override;
	virtual int							GetClassType() const override { return CLASS_SHERO; }

private:
	
	eVec2 oldFacingDirection			= vec2_oneZero;
	const int xSpeedParameterHash		= std::hash<std::string>()("xSpeed");
	const int ySpeedParameterHash		= std::hash<std::string>()("ySpeed");
	const int magnitudeParameterHash	= std::hash<std::string>()("magnitude");
};

#endif /* EVIL_SHERO_H */