#include "Tile.h"
#include "Game.h"

// FIXME: these should be class static
int									numTileTypes = 0;
std::shared_ptr<eImageTiler>		tileSet;
eTileImpl							tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::eTileImpl
// returns false on failure to init, true otherwise
//************
bool eTileImpl::InitTileTypes(const char * tilerFilename) {
	// load the tile file
	if (!game.GetImageTilerManager().LoadTiler(tilerFilename, tileSet))
		return false;

	numTileTypes = tileSet->GetNumSequences();
	if (!numTileTypes)
		return false;

	// configure the tileTypes array according to each sequence in tileSet
	for (int i = 0; i < numTileTypes; i++) {
		std::string name = tileSet->GetSequenceName(i);
		tileTypes[i].name = name;
		tileTypes[i].type = tileSet->GetFirstIndex(name);

		if (tileTypes[i].type == invalidTileType)
			return false;

		// TODO: script this somewhere, possibly in another file type
		// that stores the .tls file to load the imageTiler
		// and then parallel data for things like collision and such
		tileTypes[i].collisionHack = tileTypes[i].type == 0 ? false : true;
	}	
	return true;
}