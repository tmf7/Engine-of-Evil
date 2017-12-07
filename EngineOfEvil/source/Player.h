#ifndef EVIL_PLAYER_H
#define EVIL_PLAYER_H

#include "Entity.h"

//*************************************************
//					ePlayer
// handles all player input controls
// this is a higher level game-logic class for testing
// rather than a generic game-engine class
// TODO(~): inherit from an eGameObject class
//*************************************************
class ePlayer : public eClass {

public:

	void					Think();
	void					Draw();
	void					DebugDraw();

	virtual int				GetClassType() const override				{ return CLASS_PLAYER; }
	virtual bool			IsClassType(int classType) const override	{ 
								if(classType == CLASS_PLAYER) 
									return true; 
								return eClass::IsClassType(classType); 
							}

private:

	bool					SelectGroup();
	void					ClearGroupSelection();

private:

	std::vector<eEntity *>	groupSelection;
	std::array<eVec2, 2>	selectionPoints;			// for drawing on-screen selection box, and conversion to worldspace for eEntity selection
	bool					beginSelection = false;
};

#endif /* EVIL_PLAYER_H */
