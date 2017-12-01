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
		result = resourceList[0];
		return false;
	}

	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');							// initial state name
	std::string initialState(buffer);
	if (!VerifyRead(read)) {
		result = resourceList[0];
		return false;
	}

	int numStateNodes = 0;												// initialize hashindex sizes
	int numTransitions = 0;
	int numIntParams = 0;
	int numFloatParams = 0;
	int numBoolParams = 0;
	int numTriggerParams = 0;
	read >> numStateNodes >> numTransitions >> numIntParams >> numFloatParams >> numBoolParams >> numTriggerParams;
	if (!VerifyRead(read)) {
		result = resourceList[0];
		return false;
	}

	result->InitHashIndexes(numStateNodes, numTransitions, numIntParams, numFloatParams, numBoolParams, numTriggerParams);

	enum class LoadState {
		CONTROLLER_PARAMETERS,
		ANIMATION_STATES,
		BLEND_STATES,
		STATE_TRANSITIONS,
		FINISHED
	};
	LoadState loadState = LoadState::CONTROLLER_PARAMETERS;

	// DEBUG: always put a major-section's closing brace '}' on a new line below the last entry
	read.ignore(std::numeric_limits<std::streamsize>::max(), '{');		// jump to Controller_Parameters {\n
	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	while (read.peek() != '}') {
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ' ');						// parameter type
		std::string parameterType(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), ' ');						// parameter name
		std::string parameterName(buffer);
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}

		// TODO: log an error if the same parameter attempts to load twice (based on its nameHash)
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
		} 

		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}
	}

	while (!read.eof() && loadState != LoadState::FINISHED) {
		read.ignore(std::numeric_limits<std::streamsize>::max(), '{');	
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		switch (loadState) {				
			case LoadState::ANIMATION_STATES: {
				while (read.peek() != '}') {
					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// state name
					std::string stateName(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// animation name
					auto & animation = game.GetAnimationManager().Get(buffer);
					if (!VerifyRead(read) || !animation->IsValid()) {
						result = resourceList[0];
						return false;
					}

					float stateSpeed = 0.0f;
					read >> stateSpeed;										// state speed
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					result->AddAnimationState(std::make_unique<eAnimationState>(stateName, animation, stateSpeed));
				}
				loadState = LoadState::BLEND_STATES;
				break;
			} 

			case LoadState::BLEND_STATES: { 
				while (read.peek() != '}') {								// adding blend states
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip past implicit "{\n"

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// state name
					std::string stateName(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					int numAnimations = 0;									// state configuration
					int animationBlendMode = 0;
					float stateSpeed = 0.0f;
					read >> numAnimations >> animationBlendMode >> stateSpeed;
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					AnimationBlendMode blendMode;
					switch (animationBlendMode) {
						case 1: blendMode = AnimationBlendMode::SIMPLE_1D; break;
						case 2: blendMode = AnimationBlendMode::FREEFORM_2D; break;
						default: blendMode = AnimationBlendMode::SIMPLE_1D; break;
					}

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// x-axis blending parameter name
					std::string xBlendParameterName(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					// DEBUG: .ectrl format demands if one blend state exists at least one float param exists
					int xParameterIndex = result->GetFloatParameterIndex(xBlendParameterName);
					if (xParameterIndex < 0)
						xParameterIndex = 0;

					float * xBlendParameter = &result->floatParameters[xParameterIndex];
					float * yBlendParameter = nullptr;

					if (blendMode == AnimationBlendMode::FREEFORM_2D) {
						memset(buffer, 0, sizeof(buffer));
						read.getline(buffer, sizeof(buffer), '\n');			// y-axis blending parameter name
						std::string yBlendParameterName(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						// DEBUG: .ectrl format demands if blendMode == FREEFORM_2D that two parameters be listed
						int yParameterIndex = result->GetFloatParameterIndex(yBlendParameterName);
						if (yParameterIndex < 0)
							yParameterIndex = 0;

						yBlendParameter = &result->floatParameters[yParameterIndex];
					}

					auto newBlendState = std::make_unique<eBlendState>(stateName, numAnimations, xBlendParameter, yBlendParameter, blendMode, stateSpeed);
					while (read.peek() != '}') {							// adding blend nodes
						read.getline(buffer, sizeof(buffer), ' ');			// animation name
						std::string animationName(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						float nodeValue_X = 0.0f;
						float nodeValue_Y = 0.0f;
						read >> nodeValue_X;
						if (blendMode == AnimationBlendMode::FREEFORM_2D)
							read >> nodeValue_Y;
						
						read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						newBlendState->AddBlendNode(animationName, nodeValue_X, nodeValue_Y);
					}

					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip past blend state delimiter "}\n"
					newBlendState->Init();
					result->AddAnimationState(std::move(newBlendState));
				}
				loadState = LoadState::STATE_TRANSITIONS;
				break; 
			}
			
			case LoadState::STATE_TRANSITIONS: { 
/*
				while (read.peek() != '}') {								// adding blend states
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip past implicit "{\n"

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// transition name
					std::string transitionName(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					bool anyState = false;									// transition occurs from any eStateNode
					read >> anyState;
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// from state
					std::string fromState(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), '\n');				// to state
					std::string toState(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					float exitTime = 0.0f;
					float offset = 0.0f;
					read >> exitTime >> offset;
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}
					
					eStateTransition newTransition(transitionName, anyState, fromStateIndex, toStateIndex, exitTime, offset);




					while (read.peek() != '}') {							// adding transition conditions
						read.getline(buffer, sizeof(buffer), ' ');			// condition type (from named controller param's type)
						std::string conditionType(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						
						// controller param name		(use to lookup its index)
						// compareString				(if-else to set COMPARE_ENUM
						// value						(getline up to '\n' for this one.... >> wont do that...)

						
						newTransition->Add_XYZ_Condition(controllerFloatIndex, compareEnum, value);		// unless trigger, then no value needed
					}

					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip past blend state delimiter "}\n"

//					result->AddTransition(std::move(newTransition));
				}
*/
				loadState = LoadState::FINISHED;
				break;
			}
		}
	}

	// register the requested animation controller
	resourceHash.Add(result->GetNameHash(), resourceList.size());
	resourceList.emplace_back(result);
	return true;
}