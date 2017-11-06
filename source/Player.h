#ifndef EVIL_PLAYER_H
#define EVIL_PLAYER_H

//#include <vector>
class eEntity;

//*************************************************
//					ePlayer
// handles all player input controls
// this is a higher level game-logic class for testing
// rather than a generic game-engine class
//*************************************************
class ePlayer {

public:

	void					Think();

private:

	void					SelectGroup(const eBounds & selectionArea);
	void					ClearGroupSelection();

private:

	std::vector<eEntity *>	groupSelection;
	bool					beginSelection = false;
};

#endif /* EVIL_PLAYER_H */
