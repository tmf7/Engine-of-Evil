#include "AnimationControllerManager.h"
#include "Game.h"

//***********************
// AnimationControllerManager::Init
//***********************
bool eAnimationControllerManager::Init() {
	// prepare the hashindex
	resourceHash.ClearAndResize(MAX_ANIMATION_CONTROLLERS);

	auto & error_animation_controller = std::make_shared<eAnimationController>("error_animation_controller", resourceList.size());	
	error_animation_controller->Init(1, 0, 0, 0, 0, 0);
	if (!error_animation_controller->AddAnimationState(std::make_unique<eAnimationState>("error_state", game.GetAnimationManager().Get(0), 1.0f)))
		return false;

	// TODO: register the error_animation_controller as the first element of resourceList
	int hashKey = resourceHash.GetHashKey(std::string("error_animation_controller"));
	resourceHash.Add(hashKey, resourceList.size());
	resourceList.emplace_back(error_animation_controller);	// default error animation controller
	return true;
}

//***********************
// eAnimationControllerManager::LoadAndGet
// DEBUG (.ectrl file format):
// DEBUG: group order is always Controller_Parameters -> Animation_States -> Blend_States -> State_Transitions
// DEBUG: all major-group closing braces '}' must happen at the start of a newline,
// while opening braces '{' are on the same line as the major-group header
// DEBUG: fill out all fields, unless the group is empty then just have a closing brace '}'
// DEBUG: if one eBlendState is in "Blend_States {", then there must be at least one float parameter in "Controller_Parameters {"
// DEBUG: if SIMPLE_1D blendMode with one controller parameter, just list the same parameter twice on a blendState
/*
imagesUsedBatchFilename\n																		(filename order matters here: first images, then animations)
animationsUsedBatchFilename\n
initialStateName\n																				(used to set initial currentState index)
numStateNodes numTransitions numFloatParams numIntParams numBoolParams numTriggerParams\n		(used initialize the eHashIndexes, numStateNodes == num Animation_States + num Blend_States)
---ANY_NUMBER_OF_NEWLINES HERE----
Controller_Parameters {\n
paramType paramName paramInitialValue\n															(always list 1 or 2 params if there's at least one Blend_state)
paramType1 paramName1 paramInitialValue1\n														(type will be: int/float/bool/trigger)
(repeat adding parameters)
}\n
---ANY_NUMBER_OF_NEWLINES HERE----
Animation_States {\n
stateName singleAnimationName stateSpeed\n
stateName1 singleAnimationName1 stateSpeed1\n
(repeat adding states)
}\n
---ANY_NUMBER_OF_NEWLINES HERE----
Blend_States {\n
{\n
stateName numAnimationsInState dimension stateSpeed\n											(dimension will be: 1 or 2 for SIMPLE_1D or FREEFORM_2D, which dictates the number of float param(s) used)
controllerFloatParam1 controllerFloatParam\n													(always list two param names here, even if repeated)
animationName controllerFloatParam1Value controllerFloatParam2Value\n							(max of 2, min of 0, but it defaults to evenly distributed values)
animationName1 controllerFloatParam1Value1 controllerFloatParam2Value1\n
(repeat adding animations)
}\n
{\n
stateName numAnimationsInState dimension stateSpeed\n
controllerFloatParam1 controllerFloatParam2\n
animationName controllerFloatParam1Value controllerFloatParam2Value\n	
animationName1 controllerFloatParam1Value1 controllerFloatParam2Value1\n	
}\n
(repeat adding states)
}\n
---ANY_NUMBER_OF_NEWLINES HERE----
State_Transitions {\n
{\n
transitionName anyStateBool fromStateName toStateName exitTimeNormalized offsetNormalized\n			(anystateBool will be: 0/1)
controllerParamType controllerParamName compareEnumName	transitionValue\n							(type will be: int/float/bool/trigger)
controllerParamType1 controllerParamName1 compareEnumName1 transitionValue1\n						(compare will be: greater/greaterEqual/less/lessEqual/equal/notEqual)
(repeat adding parameter-value-compare tuples)
}\n
{\n
transitionName1 anyStateBool1 fromStateName1 toStateName1 exitTimeNormalized1 offsetNormalized1\n
controllerParamType controllerParamName compareEnumName	transitionValue\n
controllerParamType1 controllerParamName1 compareEnumName1 transitionValue\n
(repeat adding parameter-value-compare tuples)
}\n
(repeat adding transitions)
}\n 
---END_OF_FILE---
*/
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

	int numStateNodes = 0;												// controller configuration
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

	result->Init(numStateNodes, numTransitions, numIntParams, numFloatParams, numBoolParams, numTriggerParams);

	enum class LoadState {
		CONTROLLER_PARAMETERS,
		ANIMATION_STATES,
		BLEND_STATES,
		STATE_TRANSITIONS,
		FINISHED
	};
	LoadState loadState = LoadState::CONTROLLER_PARAMETERS;
	int defaultFloatNameHash = 0;
	bool firstFloatNameHashSaved = false;

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
			if (!firstFloatNameHashSaved) {								// saved first hashkey to use for blendState default parameters, if needed
				firstFloatNameHashSaved = true;
				defaultFloatNameHash = result->floatParamsHash.GetHashKey(parameterName);
			}
		} else if (parameterType == "bool") {
			bool initialBoolValue = false;
			read >> initialBoolValue;
			result->AddBoolParameter(parameterName, initialBoolValue);
		} else if (parameterType == "trigger") {
			bool initialTriggerValue = false;
			read >> initialTriggerValue;
			result->AddTriggerParameter(parameterName, initialTriggerValue);
		} 

		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (!VerifyRead(read)) {
			result = resourceList[0];
			return false;
		}
	}

	loadState = LoadState::ANIMATION_STATES;

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
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

					// DEBUG: .ectrl format demands that if one blend state exists, then at least one float param exists
					int xBlendParameterHash = result->floatParamsHash.GetHashKey(xBlendParameterName);
					int xBlendParameterIndex = result->GetFloatParameterIndex(xBlendParameterName);
					if (xBlendParameterIndex < 0)							// invalid parameter name, use default
						xBlendParameterHash = defaultFloatNameHash;


					int yBlendParameterHash = defaultFloatNameHash;
					if (blendMode == AnimationBlendMode::FREEFORM_2D) {
						memset(buffer, 0, sizeof(buffer));
						read.getline(buffer, sizeof(buffer), '\n');			// y-axis blending parameter name
						std::string yBlendParameterName(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						// DEBUG: .ectrl format demands if blendMode == FREEFORM_2D that two parameters be listed
						int yBlendParameterIndex = result->GetFloatParameterIndex(yBlendParameterName);
						yBlendParameterHash = result->floatParamsHash.GetHashKey(yBlendParameterName);
						if (yBlendParameterIndex < 0)
							yBlendParameterHash = defaultFloatNameHash;
					}


					// TODO: if no floats values are listed (just an animationName\n) then default values to 
					// (1.0f / numAnimations) * currentAnimationLoadedCount so they are evenly distributed
					// OR: add a "distribute" boolean at the top of the blend state file-definition
					// to indicate how to affect/ignore the values in the file
					auto & newBlendState = std::make_unique<eBlendState>(stateName, numAnimations, xBlendParameterHash, yBlendParameterHash, blendMode, stateSpeed);
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
						if (!newBlendState->AddBlendNode(animationName, nodeValue_X, nodeValue_Y))
							return false;									// bad animation name	// TODO: just log an error and let the error_animation play
					}

					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip past blend state delimiter "}\n"
					newBlendState->Init();
					// FIXME(?): blendNodesHash ClearAndResize to numAnimations may cause too many collisions
					result->AddAnimationState(std::move(newBlendState));
				}
				loadState = LoadState::STATE_TRANSITIONS;
				break; 
			}
			
			case LoadState::STATE_TRANSITIONS: { 
				while (read.peek() != '}') {								// adding blend states
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');		// skip past implicit "{\n"

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// transition name
					std::string transitionName(buffer);
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

					// fromState is allow to be invalid for anyState == true
					int fromStateIndex = result->GetStateIndex(fromState);

					memset(buffer, 0, sizeof(buffer));
					read.getline(buffer, sizeof(buffer), ' ');				// to state
					std::string toState(buffer);
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}

					// skip this transition if its toState is invalid
					int toStateIndex = result->GetStateIndex(toState);
					if (toStateIndex < 0) {
						read.ignore(std::numeric_limits<std::streamsize>::max(), '}');
						read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						continue;
					}

					float exitTime = 0.0f;									// when to check conditions (in normalizedTime)
					float offset = 0.0f;									// where to start toState (in normalizedTime)
					bool anyState = false;									// transition occurs from any eStateNode
					read >> anyState >> exitTime >> offset;
					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					if (!VerifyRead(read)) {
						result = resourceList[0];
						return false;
					}
					
					// skip this transition if its fromState is invalid
					if (!anyState && fromStateIndex < 0) {
						read.ignore(std::numeric_limits<std::streamsize>::max(), '}');
						read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						continue;
					}

					if (offset < 0.0f)
						offset = 0.0f;

					eStateTransition newTransition(transitionName, anyState, fromStateIndex, toStateIndex, exitTime, offset);
					while (read.peek() != '}') {							// adding transition conditions
						read.getline(buffer, sizeof(buffer), ' ');			// condition type (from named controller param's type)
						std::string conditionType(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						read.getline(buffer, sizeof(buffer), ' ');			// controller parameter name
						std::string controllerParameterName(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						read.getline(buffer, sizeof(buffer), ' ');			// comparision operator name
						std::string compareOperatorName(buffer);
						if (!VerifyRead(read)) {
							result = resourceList[0];
							return false;
						}

						// DEBUG: compareEnum will be ignored for bools and triggers because they're always COMPARE_ENUM::EQUAL
						COMPARE_ENUM compareEnum;
						if (compareOperatorName == "greater")			compareEnum = COMPARE_ENUM::GREATER;
						else if (compareOperatorName == "greaterEqual") compareEnum = COMPARE_ENUM::GREATER_EQUAL;
						else if (compareOperatorName == "less")			compareEnum = COMPARE_ENUM::LESS;
						else if (compareOperatorName == "lessEqual")	compareEnum = COMPARE_ENUM::LESS_EQUAL;
						else if (compareOperatorName == "equal")		compareEnum = COMPARE_ENUM::EQUAL;
						else if (compareOperatorName == "notEqual")		compareEnum = COMPARE_ENUM::NOT_EQUAL;

						if (conditionType == "int") {						// condition value, and validating parameter names

							const int controllerIntIndex = result->GetIntParameterIndex(controllerParameterName);
							if (controllerIntIndex >= 0) {
								int intValue = 0;
								read >> intValue;
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
								if (!VerifyRead(read)) {
									result = resourceList[0];
									return false;
								}

								newTransition.AddIntCondition(controllerIntIndex, compareEnum, intValue);
							} else {
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
							}

						} else if (conditionType == "float") {

							const int controllerFloatIndex = result->GetFloatParameterIndex(controllerParameterName);
							if (controllerFloatIndex >= 0) {
								float floatValue = 0.0f;
								read >> floatValue;
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
								if (!VerifyRead(read)) {
									result = resourceList[0];
									return false;
								}

								newTransition.AddFloatCondition(controllerFloatIndex, compareEnum, floatValue);
							} else {
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
							}

						} else if (conditionType == "bool") {

							const int controllerBoolIndex = result->GetBoolParameterIndex(controllerParameterName);
							if (controllerBoolIndex >= 0) {	
								bool boolValue = false;
								read >> boolValue;
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
								if (!VerifyRead(read)) {
									result = resourceList[0];
									return false;
								}

								newTransition.AddBoolCondition(controllerBoolIndex, boolValue);
							} else {
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
							}

						} else if (conditionType == "trigger") {

							const int controllerTriggerIndex = result->GetBoolParameterIndex(controllerParameterName);
							if (controllerTriggerIndex >= 0)
								newTransition.AddTriggerCondition(controllerTriggerIndex);
							else
								read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						} 
					}

					read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');	// skip past blend state delimiter "}\n"
					result->AddTransition(std::move(newTransition));
				}
				result->SortAndHashTransitions();
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