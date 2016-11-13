#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>		// for memset
#include <stdio.h>			// for rand
#include "Math.h"

#define BIT(a) (1<<a)
typedef unsigned char byte_t;

#define MAX_FILE_PATH 128
#define MAX_MAP_ROWS 35//256
#define MAX_MAP_COLUMNS 10//256
#define MAX_ENTITIES 4096
#define MAX_IMAGES 1024
//#define MAX_WAYPOINTS 1024		// not counting targeted entities (those are POINTERS to other entities' origins)
								// FIXME: this may not be necessary once dynamic memory is fully implemented

//#define ONE_GIGABYTE 1073741824
//extern byte_t memoryPool[ONE_GIGABYTE];

template<class type, int rows, int columns>
class eSpatialIndexGrid;

typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> ai_map_t;
typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> game_map_t;

#endif /* EVIL_DEFINITIONS_H */
