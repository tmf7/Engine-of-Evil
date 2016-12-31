#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include <cstring>			// memset
#include <ios>
#include <fstream>
#include <random>
#include <limits>
//#include <stdio.h>
#include "Math.h"

#define BIT(a) (1<<a)

#define MAX_ESTRING_LENGTH 128
#define MAX_MAP_ROWS 256
#define MAX_MAP_COLUMNS 256
#define MAX_ENTITIES 4096
#define MAX_TEXTURES 1024

//#define ONE_GIGABYTE 1073741824
//extern byte_t memoryPool[ONE_GIGABYTE];

#endif /* EVIL_DEFINITIONS_H */
