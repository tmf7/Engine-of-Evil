#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

#include "SDL.h"
#include "SDL_ttf.h"
#include <Windows.h>		// for rand()
#include <stdio.h>			// FIXME: necessary at this point?

#define BIT(a) (1<<a)
typedef unsigned char byte_t;

// FIXME: reduce the scope of these somehow?
#define MAX_MAP_ROWS 256
#define MAX_MAP_COLUMNS 256
#define MAX_ENTITIES 4096

template<class type, int rows, int columns>
class SpatialIndexGrid;

typedef SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> ai_map_t;
typedef SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS> game_map_t;

#endif /* EVIL_DEFINITIONS_H */
