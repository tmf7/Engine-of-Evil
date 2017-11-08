#ifndef EVIL_PLAYER_H
#define EVIL_PLAYER_H

#include "Entity.h"

//*************************************************
//					ePlayer
// handles all player input controls
// this is a higher level game-logic class for testing
// rather than a generic game-engine class
// TODO: inherit from an eGameObject class
//*************************************************
class ePlayer : public eClass {

public:

	void					Think();
	void					Draw();
	void					DebugDraw();

	virtual int				GetClassType() const override { return CLASS_PLAYER; }

private:

	void					SelectGroup(const eBounds & selectionArea);
	void					ClearGroupSelection();

private:

	std::vector<eEntity *>	groupSelection;
	std::array<eVec2, 2>	selectionPoints;
	bool					beginSelection = false;
};

#endif /* EVIL_PLAYER_H */
