#pragma once

/*
// TODO: only eAnimationControllerManager should be able to access eAnimationController::AddTransition and eAnimationController::AddState 
// [NOTE]: at least one float must be selected during BlendState definition, if not in load-file, then default to first floatParameter in controller
// TODO(!): don't allow duplicate animations in the same BlendState...it messes with the blendnodes indexing a bit

// [[IMPORTANT]] resourceFilename will be the .eimg file path (with contained .png path and subframe data)
// FIXME(!): what does that mean for, say, .anim files that index by image name....just change the extensions from .png to .eimg

CONTROLLER SETUP:
-----------------
1) read the file for controller params: name type initialValue 
1.5) use type string via if-else to read initialValue as intended type
[NOTE] if the type is not int/float/bool/trigger, then the param is invalid ==> unable to add, so skip the rest of the param def
2) call eAnimationController::Add_TYPE_Parameter(name, initialValue)
3) repeat for all controller params

ANIMATION_STATES SETUP:
-------------
4) read the file for animation states: stateName animationName stateSpeed stateLoop 
[NOTE] call eAnimationManager::GetAnimation(animationName, animationPtr),
if animationPtr errors because animationName doesn't exist, the state is invalid ==> LOAD-TIME FAILURE (for now)
5) call eAnimationController::AddState(eStateNode &&)
6) repeat for all animation states

BLEND_STATES SETUP:
-------------
7) read the file for blend states: stateName numAnimations stateSpeed stateLoop
read the file for which controller params to watch: controllerParamNameFromEarlier_1 controllerParamNameFromEarlier_2 (always float_type, max 2)
[NOTE] at least one controller float must be selected, if not in load-file, then default to first floatParameter
read the file for eAnimations and their associated threshold values: animationName floatValue_1 floatValue_2\n [repeat] (always float_type, max 2 parallel to controllerParams used)
[NOTE] if no floats are listed (just an animationName\n) then default to (1.0f / numAnimations) * currentAnimationLoadedCount
[NOTE] call eAnimationManager::GetAnimation(animationName, animationPtr),
if animationPtr errors because animationName doesn't exist, the state is invalid ==> LOAD-TIME FAILURE (for now)
8) call eAnimationController::AddState(eStateNode &&)
9) repeat for all blend states

TRANSITIONS SETUP:
------------------
10) read the file for transitions: anyStateBool fromStateInt toStateInt name\n
[NOTE] if eAnimationController::GetAnimationState(fromStateInt) or (toStateInt) doesn't exsit, the transition is invalid ==> LOAD-TIME FAILURE
10.5) read the file for transition params: controllerParamNameFromEarlier type transitionValue compareEnumString (type will be int/float/bool/trigger)
[NOTE] if eAnimationController::Get_TYPE_Parameter(controllerParamNameFromEarlier) doesn't exist ==> unable to add, so skip the rest of the param def
11) call eTransition::AddCondition_TYPE_(controllerParamNameFromEarlier, transitionValue, compareEnum) using if-else(type) as before
11.5) also convert StringToEnum(compareEnumString) via if-else string comparison
[NOTE] if the type is not int/float/bool/trigger, then the param is invalid ==> unable to add, so skip the rest of the param def
[NOTE] if the type is bool, skip the rest of the param def (compare is always ==)
12) call eAnimationController::AddTransition(eTransition &&)
13) repeat for all transitions
*/