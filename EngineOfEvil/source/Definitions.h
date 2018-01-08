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
#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

/*
// TODO: if very large random numbers are needed ( ie greater than RAND_MAX 32,767 )
#include <random>
	srand(SDL_GetTicks());
	std::random_device r;
	std::default_random_engine engine(r());	// seed the mersenne twister
	std::uniform_int_distribution<int> uniform_dist(0, NUM_ELEMENTS);
	int r = uniform_dist(engine) % NUM_ELEMENTS;
*/

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <cstring>			// memset
#include <memory>			// std::unique_ptr, std::shared_ptr, std::weak_ptr
#include <string>
#include <ios>
#include <fstream>
#include <random>
#include <limits>
#include <unordered_map>
#include <deque>
#include <vector>
#include <array>
#include <functional>		// std::hash
#include <regex>
#include "Sort.h"
#include "Math.h"
#include "ErrorLogger.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)
#define TO_STRING(x) (#x)
#define BIT(a) (1<<a)
#define XOR_SEQ(a,b) (a) ? !(b) : (b)
#define XOR(a,b) !(a) != !(b)

#define MAX_ESTRING_LENGTH 128
#define MAX_MAP_ROWS 256
#define MAX_MAP_COLUMNS 256
#define MAX_ENTITIES 4096
#define MAX_PREFAB_ENTITIES 1024
#define MAX_IMAGES 1024
#define MAX_ANIMATIONS 1024
#define MAX_ANIMATION_CONTROLLERS 512
#define MAX_LAYER 128
#define INVALID_ID -1

/*
#define REGISTER_ENUM(x) x,
typedef enum {
	#include "ClassTypes.h"
	CLASS_INVALID
} ClassTypes_t;
#undef REGISTER_ENUM
*/

#endif /* EVIL_DEFINITIONS_H */
