#include "AnimationManager.h"
#include "Game.h"

//***************
// eAnimationManager::Init
//***************
bool eAnimationManager::Init() {
	// prepare the hashindex
	animationFilenameHash.ClearAndResize(MAX_IMAGES);

	// get the error_image, a bright red texture the size of the current render target
	// to use as the error_animation source
	std::shared_ptr<eImage> error_image;
	game.GetImageManager().GetImage(0, error_image);

	// generate an single-sequence sequenceHash (with one name)
	// and frameList of one frame the size of the error_image
	auto hasher = std::hash<std::string>{};
	std::vector<std::string> sequenceNames;
	eHashIndex sequenceHash;
	std::vector<eAnimationFrame> frameList;
	sequenceNames.reserve(1);
	frameList.reserve(1);
	sequenceHash.ClearAndResize(1);
	std::string error_name = "error_animation";
	sequenceNames.push_back(error_name);
	sequenceHash.Add(hasher(error_name), 0);
	frameList.push_back(eAnimationFrame( SDL_Rect{ 0, 0, error_image->GetWidth(), error_image->GetHeight() } , frameList.data()));

	// register the error_animation as the first element of animationList
	animationFilenameHash.Add(hasher(error_name), animationList.size());
	animationList.push_back(std::make_shared<eAnimation>(error_image, std::move(frameList), std::move(sequenceNames), std::move(sequenceHash), "invalid_file", animationList.size()));
	return true;
}

//***************
// eAnimationManager::BatchLoad
// loads a batch of eAnimation resources
// user can optionally call animationManager.Clear()
// prior to this to facilitate starting with a fresh set of eAnimations
// TODO: allow selective unloading of animations (eg: std::shared_ptr already does reference counting
// take those numbers and add/subtract according to the next level's filename batch)
//***************
bool eAnimationManager::BatchLoad(const char * animationBatchLoadFile) {
	std::shared_ptr<eAnimation> result;	// DEBUG: not acually used, but necessary for LoadAnimation
	char filename[MAX_ESTRING_LENGTH];
	std::ifstream	read(animationBatchLoadFile);

	// unable to find/open file
	if(!read.good())
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the first line of the file
	while (!read.eof()) {
		read >> filename;
		if (!VerifyRead(read))
			return false;

		LoadAnimation(filename, result);
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//***************
// eAnimationManager::GetAnimation
// fill the result with an eImage * if it exists
// if filename is null or the eImage doesn't exist
// then it result fills with an error eImage * (solid red)
//***************
bool eAnimationManager::GetAnimation(const char * filename, std::shared_ptr<eAnimation> & result) {
	if (!filename) {
		result = animationList[0]; // error eAnimation
		return false;
	}

	// search for pre-existing texture
	auto hasher = std::hash<std::string>{};
	int hashkey = hasher(filename);
	for (int i = animationFilenameHash.First(hashkey); i != -1; i = animationFilenameHash.Next(i)) {
		if (animationList[i]->GetFilename() == filename) {
			result = animationList[i];
			return true;
		}
	}
	result = animationList[0]; // error eAnimation
	return false;
}

//***************
// eAnimationManager::GetAnimation
// fill the result with an eAnimation pointer if it exists
// if guid is negative or beyond the number of loaded images
// then it result fills with an error eAnimation pointer (solid red)
//***************
bool eAnimationManager::GetAnimation(int animationID, std::shared_ptr<eAnimation> & result) {
	if (animationID < 0 && animationID > animationList.size()) {		// DEBUG: animationID will never be larger than max signed int
		result = animationList[0]; // error eAnimation
		return false;
	}
	result = animationList[animationID];
	return true;
}

//***************
// eAnimationManager::LoadAnimation
// sets result to an existing or
// loaded eAnimation pointer
//***************
bool eAnimationManager::LoadAnimation(const char * filename, std::shared_ptr<eAnimation> & result) {
	// check if the eAnimation already exists 
	// and set result to that if it does
	if (GetAnimation(filename, result))
		return true;

	char buffer[MAX_ESTRING_LENGTH];
	std::vector<std::string> sequenceNames;
	eHashIndex sequenceHash;
	std::vector<eAnimationFrame> frameList;
	std::shared_ptr<eImage> source = nullptr;
	auto hasher = std::hash<std::string>{};

	std::ifstream	read(filename);
	// unable to find/open file
	if (!read.good()) {
		result = animationList[0];		// error eAnimation
		return false;
	}

	// read the source image name
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if(!VerifyRead(read)) {
		result = animationList[0];	// error eAnimation
		return false;
	}

	// get a pointer to the source image (or try to load it if it doesn't exist yet)
	if (!game.GetImageManager().LoadImage(buffer, SDL_TEXTUREACCESS_STATIC, source)) {
		result = animationList[0];	// error eAnimation
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
			result = animationList[0];	// error eAnimation
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
			result = animationList[0];	// error eAnimation
			return false;
		}

		// add the sequence name and index to the hash
		sequenceNames.push_back(buffer);
		sequenceHash.Add(hasher(buffer), frameList.size());
		eAnimationFrame * firstFrame = frameList.data() + frameList.size();		// used to close sequence loops

		// read and link one frame at at time
		// until the sequence-closing delimeter
		eAnimationFrame * nextFrame = nullptr;
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
					result = animationList[0];	// error eAnimation
					return false;
				}
			}

			// close or continue the frame sequence loop 
			// DEBUG: a single-frame sequence just points back to itself
			nextFrame = (read.peek() != '}' && frameList.size() + 1 < frameList.capacity())
						? frameList.data() + frameList.size() + 1
						: firstFrame;
			frameList.push_back(eAnimationFrame(frame, nextFrame));
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the sequence line
	}
	read.close();

	// register the requested eAnimation
	animationFilenameHash.Add(hasher(filename), animationList.size());
	result = std::make_shared<eAnimation>(source, std::move(frameList), std::move(sequenceNames), std::move(sequenceHash), filename, animationList.size());
	animationList.push_back(result);
	return true;
}

//***************
// eAnimationManager::Clear
// clears all pointers to the current set 
// of resource eAnimations, which allows them
// to be deleted once no object is using them,
// allows for new resource images to load
// without using excessive memory
//***************
void eAnimationManager::Clear() {
	animationList.clear();
	animationFilenameHash.Clear();
}