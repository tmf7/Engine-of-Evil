#include "ImageTilerManager.h"
#include "Game.h"

//***************
// eImageTilerManager::Init
//***************
bool eImageTilerManager::Init() {
	// prepare the hashindex
	tilerFilenameHash.ClearAndResize(MAX_IMAGES);

	// get the error_image, a bright red texture the size of the current render target
	// to use as the error_imageTiler source
	std::shared_ptr<eImage> error_image;
	game.GetImageManager().GetImage(0, error_image);

	// generate an single-sequence sequenceHash (with one name)
	// and frameList of one frame the size of the error_image
	auto hasher = std::hash<std::string>{};
	std::vector<std::string> sequenceNames;
	eHashIndex sequenceHash;
	std::vector<eImageFrame> frameList;
	sequenceNames.reserve(1);
	frameList.reserve(1);
	sequenceHash.ClearAndResize(1);
	std::string error_name = "error_imageTiler";
	sequenceNames.push_back(error_name);
	sequenceHash.Add(hasher(error_name), 0);
	frameList.push_back(eImageFrame( SDL_Rect{ 0, 0, error_image->GetWidth(), error_image->GetHeight() } , frameList.data()));

	// register the error_imageTiler as the first element of tilerList
	tilerFilenameHash.Add(hasher(error_name), tilerList.size());
	tilerList.push_back(std::make_shared<eImageTiler>(error_image, std::move(frameList), std::move(sequenceNames), std::move(sequenceHash), "invalid_file", tilerList.size()));
	globalIDPool++;
	return true;
}

//***************
// eImageTilerManager::BatchLoad
// loads a batch of imageTiler resources
// user can optionally call imageTilerManager.Clear()
// prior to this to facilitate starting with a fresh set of imageTilers
// TODO: allow selective unloading of tilers (eg: std::shared_ptr already does reference counting
// take those numbers and add/subtract according to the next level's filename batch)
//***************
bool eImageTilerManager::BatchLoad(const char * tilerBatchLoadFile) {
	std::shared_ptr<eImageTiler> result;	// DEBUG: not acually used, but necessary for LoadTiler
	char filename[MAX_ESTRING_LENGTH];
	std::ifstream	read(tilerBatchLoadFile);

	// unable to find/open file
	if(!read.good())
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the first line of the file
	while (!read.eof()) {
		read >> filename;
		if (!VerifyRead(read))
			return false;

		LoadTiler(filename, result);
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//***************
// eImageTilerManager::GetTiler
// fill the result with an eImage * if it exists
// if filename is null or the eImage doesn't exist
// then it result fills with an error eImage * (solid red)
//***************
bool eImageTilerManager::GetTiler(const char * filename, std::shared_ptr<eImageTiler> & result) {
	if (!filename) {
		result = tilerList[0]; // error imageTiler
		return false;
	}

	// search for pre-existing texture
	auto hasher = std::hash<std::string>{};
	int hashkey = hasher(filename);
	for (int i = tilerFilenameHash.First(hashkey); i != -1; i = tilerFilenameHash.Next(i)) {
		if (tilerList[i]->GetFilename() == filename) {
			result = tilerList[i];
			return true;
		}
	}
	result = tilerList[0]; // error imageTiler
	return false;
}

//***************
// eImageTilerManager::GetTiler
// fill the result with an eImageTiler pointer if it exists
// if guid is negative or beyond the number of loaded images
// then it result fills with an error eImageTiler pointer (solid red)
//***************
bool eImageTilerManager::GetTiler(int tilerID, std::shared_ptr<eImageTiler> & result) {
	if (tilerID < 0 && tilerID > tilerList.size()) {		// DEBUG: tilerID will never be larger than max signed int
		result = tilerList[0]; // error imageTiler
		return false;
	}
	result = tilerList[tilerID];
	return true;
}

//***************
// eImageTilerManager::LoadTiler
// sets result to an existing or
// loaded eImageTiler pointer
//***************
bool eImageTilerManager::LoadTiler(const char * filename, std::shared_ptr<eImageTiler> & result) {
	// check if the imageTiler already exists 
	// and set result to that if it does
	if (GetTiler(filename, result))
		return true;

	char buffer[MAX_ESTRING_LENGTH];
	std::vector<std::string> sequenceNames;
	eHashIndex sequenceHash;
	std::vector<eImageFrame> frameList;
	std::shared_ptr<eImage> source = nullptr;
	auto hasher = std::hash<std::string>{};

	std::ifstream	read(filename);
	// unable to find/open file
	if (!read.good()) {
		result = tilerList[0];		// error imageTiler
		return false;
	}

	// read the source image name
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if(!VerifyRead(read)) {
		result = tilerList[0];	// error imageTiler
		return false;
	}

	// get a pointer to the source image (or try to load it if it doesn't exist yet)
	if (!game.GetImageManager().LoadImage(buffer, SDL_TEXTUREACCESS_STATIC, source)) {
		result = tilerList[0];	// error imageTiler
		return false;
	}

	// read frameList and sequenceHash sizes
	int numFrames;
	int numSequences;
	for (int targetData = 0; targetData < 2; targetData++) {
		switch (targetData) {
			case 0: read >> numFrames; break;
			case 1:	read >> numSequences; break;
			default: break;
		}
		if (!VerifyRead(read)) {
			result = tilerList[0];	// error imageTiler
			return false;
		}
	}
	frameList.reserve(numFrames);					// minimize dynamic allocations
	sequenceNames.reserve(numSequences);
	sequenceHash.ClearAndResize(numSequences);		// esure each sequenceName has a unique hash

	while (!read.eof()) {
		// read a sequence name
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '{');
		if (!VerifyRead(read)) {
			result = tilerList[0];	// error imageTiler
			return false;
		}

		// add the sequence name and index to the hash
		sequenceNames.push_back(buffer);
		sequenceHash.Add(hasher(buffer), frameList.size());
		eImageFrame * firstFrame = frameList.data() + frameList.size();		// used to close sequence loops

		// read and link one frame at at time
		// until the sequence-closing delimeter
		eImageFrame * nextFrame = nullptr;
		while (read.peek() != '}' && nextFrame != firstFrame) {
			SDL_Rect frame;
			for (int targetData = 0; targetData < 4; targetData++) {
				switch (targetData) {
					case 0: read >> frame.x; break;
					case 1: read >> frame.y; break;
					case 2: read >> frame.w; break;
					case 3: read >> frame.h; break;
					default: break;
				}
		
				if (!VerifyRead(read)) {
					result = tilerList[0];	// error imageTiler
					return false;
				}
			}

			// close or continue the frame sequence loop 
			// DEBUG: a single-frame sequence just points back to itself
			nextFrame = (read.peek() != '}' && frameList.size() + 1 < frameList.capacity())
						? frameList.data() + frameList.size() + 1
						: firstFrame;
			frameList.push_back(eImageFrame(frame, nextFrame));
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the sequence line
	}
	read.close();

	// register the requested imageTiler
	tilerFilenameHash.Add(hasher(filename), tilerList.size());
	result = std::make_shared<eImageTiler>(source, std::move(frameList), std::move(sequenceNames), std::move(sequenceHash), filename, tilerList.size());
	tilerList.push_back(result);
	globalIDPool++;
	return true;
}

//***************
// eImageTilerManager::Clear
// clears all pointers to the current set 
// of resource tilers, which allows them
// to be deleted once no object is using them,
// allows for new resource images to load
// without using excessive memory
//***************
void eImageTilerManager::Clear() {
	tilerList.clear();
	tilerFilenameHash.Clear();
}