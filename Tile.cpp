#include "Tile.h"
#include "Game.h"

// FIXME: don't use these global non-const variables
int			numTileTypes = 0;
eImage *	tileSet = nullptr;
eTileImpl	tileTypes[eTileImpl::maxTileTypes];

//************
// eTileImpl::eTileImpl
// returns false on failure to init, true otherwise
//************
bool eTileImpl::InitTileTypes(const char * tileSetImageFile, const char * tileFormatFile) {
	char buffer[MAX_ESTRING_LENGTH];
	char tileName[MAX_ESTRING_LENGTH];

	// load the tile file
	tileSet = game.GetImageManager().GetImage(tileSetImageFile, nullptr);
	if (tileSet == nullptr)
		return false;

	std::ifstream	read(tileFormatFile);
	// unable to find/open file
	if(!read.good())
		return false;

	int typeIndex = 0;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the first line of the file
	while (!read.eof()) {
		tileTypes[typeIndex].type = typeIndex;

		// read and set the tile name
		// EG: "graphics/tiles.bmp_brick"
		memset(tileName, 0, sizeof(tileName));
		strcpy_s(tileName, tileSetImageFile);
		strcat_s(tileName, "_");

		read.getline(buffer, sizeof(buffer), '\t');
		// unrecoverable read error or improperly formatted file
		if (read.bad() || read.fail()) {
			read.clear();
			read.close();
			return false;
		}
		strncat_s(tileName, buffer, read.gcount());

		// initialize the image
		tileTypes[typeIndex].tileImage.Init(tileSet->Source(), tileName);
		
		// read the tile frame data
		// DEBUG: as well as hack collision and hack zDepth data
		SDL_Rect & targetFrame = tileTypes[typeIndex].tileImage.Frame();
		for (int targetData = 0; targetData < 6; targetData++) {
			switch (targetData) {
				case 0: read >> targetFrame.x; break;
				case 1: read >> targetFrame.y; break;
				case 2: read >> targetFrame.w; break;
				case 3: read >> targetFrame.h; break;
				case 4: read >> tileTypes[typeIndex].collisionHack; break;
				default: break;
			}
		
			// unrecoverable read error or improperly formatted file
			if (read.bad() || read.fail()) {
				read.clear();
				read.close();
				return false;
			}
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
		typeIndex++;
	}
	numTileTypes = typeIndex;
	read.close();
	return true;
}