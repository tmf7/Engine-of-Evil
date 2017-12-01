#include "AnimationManager.h"
#include "Game.h"

//***********************
// eAnimationManager::Init
//***********************
bool eAnimationManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_ANIMATIONS);

	// register the error_animation as the first element of resourceList
	// a single frame of animation using the default error image that plays one frame forever
	std::vector<AnimationFrame_t> oneDefaultAnimationFrame(1);
	int hashKey = resourceHash.GetHashKey(std::string("error_animation"));
	resourceHash.Add(hashKey, resourceList.size());
	resourceList.emplace_back(std::make_shared<eAnimation>("error_animation", 0, oneDefaultAnimationFrame, 1));	// default error animation
	return true;
}

//***********************
// eAnimationManager::LoadAndGet
// DEBUG (.eanim file format):
// numTotalAnimationFrames framesPerSecond loopMode\n	(int int int, where the third int == 1 for ONCE and 2 for REPEAT)
// imageFilepath {\n	(defines the eImageManager::resourceList index to use, by name, for the frames that follow; '{' is the delimiter)
// imageSubframeIndex normalizedTime\t\t# frameNumber comment\n
// imageSubframeIndex1 normalizedTim12\t\t# frameNumber1 comment\n
// (repeat for all subframes used for this image file)
// }\n				(indicates end of all subframe indexes of the above image to use)
// imageFilepath2 {\n
// imageSubframeIndexN normalizedTimeN\t\t# frameNumberN comment\n
// imageSubframeIndexN1 normalizedTimeN1\t\t# frameNumberN1 comment\n
// (repeat for all subframes used for this image file)
// }\n
// [NOTE]: imageFilepaths (and subframes) can be repeated if the same image is needed at discontinuous parts of the animation
// [NOTE]: the output simply defines pairs of ints that grab images and subframes at runtime (using eImageManager and eImage, respectively)
// [NOTE]: batch animation files are .banim
//***********************
bool eAnimationManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eAnimation> & result) {
	// animation already loaded
	if ((result = Get(resourceFilename))->IsValid())
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) {
		result = resourceList[0];				// default error animation
		return false;
	}

	int numAnimationFrames = 0;
	int framesPerSecond = 0;
	int loopInt = 0;
	AnimationLoopState loopMode;

	read >> numAnimationFrames >> framesPerSecond >> loopInt;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip rest of the line comment
	if(!VerifyRead(read)) {
		result = resourceList[0];				// default error animation
		return false;
	}

	switch(loopInt) {
		case 1: loopMode = AnimationLoopState::ONCE; break;
		case 2: loopMode = AnimationLoopState::REPEAT; break;
		default: loopMode = AnimationLoopState::REPEAT; break;
	}

	static std::vector<AnimationFrame_t> animationFrames;				// static to reduce dynamic allocations
	animationFrames.clear();											// lazy clearing
//	animationFrames.reserve(numAnimationFrames);						// DEBUG: commented out to take advantage of exponential growth if needed

	while (!read.eof()) {
		char imageFilepath[MAX_ESTRING_LENGTH];
		memset(imageFilepath, 0, sizeof(imageFilepath));
		read.getline(imageFilepath, sizeof(imageFilepath), '{');
		if(!VerifyRead(read)){
			result = resourceList[0];			// default error animation
			return false;
		}
		
		const int imageManagerIndex = game.GetImageManager().Get(imageFilepath)->GetManagerIndex();

		while (!read.eof() && read.peek() != '}') {
			AnimationFrame_t newAnimFrame;
			newAnimFrame.imageManagerIndex = imageManagerIndex;
			read >> newAnimFrame.subframeIndex >> newAnimFrame.normalizedTime;
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if(!VerifyRead(read)){
				result = resourceList[0];		// default error animation
				return false;
			}

			animationFrames.emplace_back(std::move(newAnimFrame));
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip the closing brace (ie: "}\n")
	}

	// register the requested animation
	result = std::make_shared<eAnimation>(resourceFilename, resourceList.size(), animationFrames, framesPerSecond, loopMode);
	resourceHash.Add(result->GetNameHash(), resourceList.size());
	resourceList.emplace_back(result);
	return true;
}