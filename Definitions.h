#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>		// for rand()
#include <stdio.h>			// FIXME: necessary at this point?

#define BIT(a) (1<<a)
typedef unsigned char byte_t;

#define MAX_FILE_PATH 128
#define MAX_MAP_ROWS 5//256
#define MAX_MAP_COLUMNS 10//256
#define MAX_ENTITIES 2		//	4096
#define MAX_IMAGES 32		//	1024

template<class type, int rows, int columns>
class eSpatialIndexGrid;

typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> ai_map_t;
typedef eSpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> game_map_t;

#endif /* EVIL_DEFINITIONS_H */
