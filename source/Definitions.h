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
#include <cstring>			// memset
#include <memory>			// std::unique_ptr, std::shared_ptr, std::weak_ptr
#include <string>
#include <ios>
#include <fstream>
#include <random>
#include <limits>
//#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <array>
#include <deque>
#include "Math.h"

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
#define MAX_IMAGES 1024
#define MAX_LAYER 128
#define INVALID_ID -1


//#define ONE_GIGABYTE 1073741824
//extern byte_t memoryPool[ONE_GIGABYTE];

//*************************************************
//					exceptions
//*************************************************
struct badEntityCtorException : public std::exception {
	badEntityCtorException(const char * entityPrefabFilename)
		: what(entityPrefabFilename) {
	};

	std::string what;
};

#endif /* EVIL_DEFINITIONS_H */
