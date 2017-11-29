#include "AnimationControllerManager.h"
#include "Game.h"

//***********************
// AnimationControllerManager::Init
//***********************
bool eAnimationControllerManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_ANIMATION_CONTROLLERS);

	// TODO: register the error_animation as the first element of resourceList
	// FIXME: the make_shared needs the proper ctor arguments
//	int hashKey = resourceHash.GetHashKey(std::string("error_animation_controller"));
//	resourceHash.Add(hashKey, resourceList.size());
//	resourceList.emplace_back(std::make_shared<eAnimationController>("error_animation_controller", 0));	// default error animation controller
	return true;
}

//***********************
// eAnimationManager::LoadAndGet
// DEBUG (.ectrl file format):
// TODO(!!): ***********define the file format here**************
// [NOTE]: batch animation files are .bctrl
//***********************
bool eAnimationControllerManager::LoadAndGet(const char * resourceFilename, std::shared_ptr<eAnimationController> & result) {
	// animation controller already loaded
	if ((result = Get(resourceFilename))->IsValid())
		return true;

	std::ifstream	read(resourceFilename);
	if (!read.good()) {
		result = resourceList[0];				// default error animation controller
		return false;
	}

	// FIXME(!): nearly everything below here needs redefinition bases on the .ectrl file format
/*
	int numAnimationFrames = 0;
	int framesPerSecond = 0;
	int loopInt = 0;
	AnimationLoopState loopMode;

	read >> numAnimationFrames >> framesPerSecond >> loopInt;
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip rest of the line comment
	if(!VerifyRead(read)) {
		result = resourceList[0];				// default error animation controller
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
		read.getline(imageFilepath, sizeof(imageFilepath), '\n');
		if(!VerifyRead(read)){
			result = resourceList[0];			// default error animation controller
			return false;
		}
		
		const int imageManagerIndex = game.GetImageManager().Get(imageFilepath)->GetManagerIndex();

		read.ignore(std::numeric_limits<std::streamsize>::max(), '{');
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		while (!read.eof() && read.peek() != '}') {
			AnimationFrame_t newAnimFrame;
			newAnimFrame.imageManagerIndex = imageManagerIndex;
			read >> newAnimFrame.subframeIndex >> newAnimFrame.normalizedTime;
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			if(!VerifyRead(read)){
				result = resourceList[0];		// default error animation controller
				return false;
			}

			animationFrames.emplace_back(std::move(newAnimFrame));
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip the closing brace (ie: "}\n")
	}

	// register the requested animation
	result = std::make_shared<eAnimationController>(resourceFilename, resourceList.size(), animationFrames, framesPerSecond, loopMode);
	resourceHash.Add(result->GetNameHash(), resourceList.size());
	resourceList.emplace_back(result);
*/
	return true;
}