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

	result = std::make_shared<eAnimationController>(resourceFilename, resourceList.size());	

	char buffer[MAX_ESTRING_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');							// image batch file
	if (!VerifyRead(read) || !game.GetImageManager().BatchLoad(buffer)) {
		result = resourceList[0];				// default error animation controller, and destroy allocated result
		return false;
	}


	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');							// animation batch file
	if (!VerifyRead(read) || !game.GetAnimationManager().BatchLoad(buffer)) {
		result = resourceList[0];				// default error animation controller, and destroy allocated result
		return false;
	}

	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');							// initial state name
	std::string initialState(buffer);
	if (!VerifyRead(read)) {
		result = resourceList[0];				// default error animation controller, and destroy allocated result
		return false;
	}

	// FIXME: what if there are no parameters (eg: the controller is just one eAnimationState)
	// SOLUTION: should be okay because the text Controller_Parameters still must exist then it could be {} or { } or {\n\n  } or {\n}
	// just make sure that logic holds here... IT DOESN'T:
	// {} works fine (won't enter loop)
	// { } will first read an empty string, then the next geline will rocket ahead to the next ' ', destrying the flow
	// {\n\n } will read a \n, then rocket ahead as before
	// {\n} whould also work fine (won't enter loop...probably)

	// [NOTE] getline extracts delimiter (advances sentry) but does not store it

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// Controller_Parameters {\n
	while (read.peek() != '}') {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ' ');						// parameter type
		std::string parameterType(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];			// default error animation controller, and destroy allocated result
			return false;
		}

		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ' ');						// parameter name
		std::string parameterName(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];			// default error animation controller, and destroy allocated result
			return false;
		}

		if (parameterType == "int") {									// parameter initial value
			int initialIntValue = 0;
			read >> initialIntValue;
			result->AddIntParameter(parameterName, initialIntValue);
		} else if (parameterType == "float") {
			float initialFloatValue = 0.0f;
			read >> initialFloatValue;
			result->AddFloatParameter(parameterName, initialFloatValue);
		} else if (parameterType == "bool") {
			bool initialBoolValue = false;
			read >> initialBoolValue;
			result->AddBoolParameter(parameterName, initialBoolValue);
		} else if (parameterType == "trigger") {
			bool initialTriggerValue = false;
			read >> initialTriggerValue;
			result->AddTriggerParameter(parameterName, initialTriggerValue);
		} else {
			// FIXME: if the parameter type is invalid, just skip it...(or error the controller?)
			// SOLUITON: seems by default nothing is added without a check...
		}


		if (!VerifyRead(read)) {
			result = resourceList[0];			// default error animation controller, and destroy allocated result
			return false;
		}
	}

	std::regex animStateRegex("(Animation_States)",  std::regex_constants::ECMAScript | std::regex_constants::icase);
	std::regex blendStateRegex("(Blend_States)",  std::regex_constants::ECMAScript | std::regex_constants::icase);
	std::regex transitionRegex("(State_Transitions)",  std::regex_constants::ECMAScript | std::regex_constants::icase);
	while (!read.eof()) {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '{');						// determine which controller component to add

		if ( std::regex_search(buffer, animStateRegex)) {				// add animation states

			while (read.peek() != '}') {
				memset(buffer, 0, sizeof(buffer));
				read.getline(buffer, sizeof(buffer), ' ');				// state name
				std::string stateName(buffer);
				if (!VerifyRead(read)) {
					result = resourceList[0];			// default error animation controller, and destroy allocated result
					return false;
				}

				memset(buffer, 0, sizeof(buffer));
				read.getline(buffer, sizeof(buffer), ' ');				// animation name
				auto & animation = game.GetAnimationManager().Get(buffer);
				if (!VerifyRead(read) || !animation->IsValid()) {
					result = resourceList[0];			// default error animation controller, and destroy  allocated result
					return false;
				}

				float stateSpeed = 0.0f;
				read >> stateSpeed;										// state speed
				read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				if (!VerifyRead(read)) {
					result = resourceList[0];			// default error animation controller, and destroy allocated result
					return false;
				}

				result->AddAnimationState(std::make_unique<eAnimationState>(stateName, animation, stateSpeed));
			}

		} else if (std::regex_search(buffer, blendStateRegex)) {		// add blend states
			// TODO: new loop until one or two '}'
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// ignore the first '{' ???, but what if: Blend_States { }



		} else if (std::regex_search(buffer, transitionRegex)) {		// add state transitions
			// TODO: new loop until one or two '}'
		}
	}

	// register the requested animation controller
	resourceHash.Add(result->GetNameHash(), resourceList.size());
	resourceList.emplace_back(result);
	return true;
}