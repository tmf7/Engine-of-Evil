/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#include "StateTransition.h"

using namespace evil;

ECLASS_DEFINITION(eClass, eStateTransition)

//*******************
// eStateTransition::eStateTransition
//*******************
eStateTransition::eStateTransition(const std::string & name, bool anyState, int fromStateIndex, int toStateIndex, float exitTime, float offset) 
	: name(name),
	  anyState(anyState),
	  fromState(fromStateIndex),
	  toState(toStateIndex),
	  exitTime(exitTime),
	  offset(offset) {
	nameHash = std::hash<std::string>()(name);
}

//*******************
// eStateTransition::AddFloatCondition
// takes the index within eAnimationController::floatParameters to compare
// to param value according to param compare
//*******************
void eStateTransition::AddFloatCondition(int controllerFloatIndex, COMPARE_ENUM compare, float value) {
	floatConditions.emplace_back(std::make_tuple(controllerFloatIndex, compare, value));
}

//*******************
// eStateTransition::AddIntCondition
// takes the index within eAnimationController::intParameters to compare
// to param value according to param compare
//*******************
void eStateTransition::AddIntCondition(int controllerIntIndex, COMPARE_ENUM compare, int value) {
	intConditions.emplace_back(std::make_tuple(controllerIntIndex, compare, value));
}

//*******************
// eStateTransition::AddBoolCondition
// takes the index within eAnimationController::boolParameters to compare
// to param value according to a "==" operation
//*******************
void eStateTransition::AddBoolCondition(int controllerBoolIndex, bool value) {
	boolConditions.emplace_back(std::make_pair(controllerBoolIndex, value));
}

//*******************
// eStateTransition::AddTriggerCondition
// takes the index within eAnimationController::triggerParameters to compare
// to "true" according to a "==" operation
//*******************
void eStateTransition::AddTriggerCondition(int controllerTriggerIndex) {
	triggerConditions.emplace_back(std::make_pair(controllerTriggerIndex, true));
}