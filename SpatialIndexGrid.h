#ifndef EVIL_SPATIAL_INDEX_GRID_H
#define EVIL_SPATIAL_INDEX_GRID_H

#include "Math.h"

//*************************************************
//				Spatial Index Grid
//	Maps points in 2D space to elements of a 2D array
//  by divinding the space into a regular grid of cells
//*************************************************

// typedef SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLS, TILE_SIZE> game_map_t;
// game_map_t tileMap;

// typedef SpatialIndexGrid<bool, MAX_MAP_ROWS, MAX_MAP_COLS, TILE_SIZE> ai_map_t;
// ai_map_t knownMap;	// for entity_1

// TODO: for each class that uses a SpatialIndexGrid: 
// #define INVALID_MAP_CELL (SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS>::INVALID_CELL)
// then if ( &result == &INVALID_MAP_CELL ) return;
// but is there a way to encapsulate that result?

template< class type, size_t rows, size_t columns>
class SpatialIndexGrid {
private:
	type					cells[rows][columns];
	size_t					cellWidth;
	size_t					cellHeight;

public:

	static const size_t		INVALID_INDEX = (size_t)-1;				// largest possible unsigned int
	static const type		INVALID_CELL;							// default constructor, one instance per grid type

							SpatialIndexGrid();
							SpatialIndexGrid(size_t cellWidth, size_t cellHeight);

	void					Index(const eVec2 & point, size_t & row, size_t & column) const;
	type *					Index(const eVec2 & point);
	type *					Index(size_t row, size_t column);
	type &					Cell(size_t row, size_t column) const;
	type &					Cell(const eVec2 & point) const;

	size_t					CellWidth() const;
	size_t					CellHeight() const;
	void					SetCellWidth(size_t cellWidth);
	void					SetCellHeight(size_t cellHeight);
};

//******************
// SpatialIndexGrid
//******************
template< class type, size_t rows, size_t columns>
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid() : cellWidth(NULL), cellHeight(NULL) {
}

//******************
// SpatialIndexGrid
//******************
template< class type, size_t rows, size_t columns>
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid(size_t cellWidth, size_t cellHeight) : cellWidth(cellWidth), cellHeight(cellHeight) {
}

//******************
// Index
// sets the reference row and column to
// grid-area-scaled values using the given point
// users must check for SpatialIndexGrid<type, rows, columns>::INVALID_INDEX return values
//******************
template< class type, size_t rows, size_t columns>
inline void SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point, size_t & row, size_t & column)  const {

	if (cellWidth == 0 || cellHeight == 0) {
		row = INVALID_INDEX;
		column = INVALID_INDEX;
		return;
	}

	row = (size_t)(point.x / cellWidth);
	column = (size_t)(point.y / cellHeight);

	if (row >= rows)
		row = INVALID_INDEX;

	if (column >= columns)
		column = INVALID_INDEX;
}

//******************
// Index
// returns a pointer to cells[r][c] closest to the given point
// to allow modification of the cell value
// users must check for nullptr return value
//******************
template< class type, size_t rows, size_t columns>
inline type * SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) {
	size_t row;
	size_t column;

	Index(point, row, column);
	if (row == INVALID_INDEX || column == INVALID_INDEX)
		return nullptr;

	return &cells[row][column];
}

//******************
// Index
// returns a pointer to cells[row][column]
// to allow modification of the cell value
// users must check for nullptr return value
//******************
template< class type, size_t rows, size_t columns>
inline type * SpatialIndexGrid<type, rows, columns>::Index(size_t row, size_t column) {

	if (row >= rows || column >= columns)
		return nullptr;

	return &cells[row][column];
}

//******************
// Cell
// returns cells[row][column] or INVALID_CELL if out of bounds
// users must check for SpatialIndexGrid<type, rows, columns>::INVALID_CELL return value
// by comparing its static address space:
// EG: #define INVALID_MAP_CELL (SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS>::INVALID_CELL)
// if ( &result == &INVALID_MAP_CELL ) return;
//******************
template< class type, size_t rows, size_t columns>
inline type & SpatialIndexGrid<type, rows, columns>::Cell(size_t row, size_t column) const {

	if (row >= rows || column >= columns)
		return INVALID_CELL;

	return cells[row][column];	// FIXME: test that this is a const reference
}

//******************
// Cell
// returns cells[r][c] closest to the given point or INVALID_CELL if out of bounds
// users must check for SpatialIndexGrid<type, rows, columns>::INVALID_CELL return values
// by comparing its static address space:
// EG: #define INVALID_MAP_CELL (SpatialIndexGrid<byte_t, MAX_MAP_ROWS, MAX_MAP_COLUMNS>::INVALID_CELL)
// if ( &result == &INVALID_MAP_CELL ) return;
//******************
template< class type, size_t rows, size_t columns>
inline type & SpatialIndexGrid<type, rows, columns>::Cell(const eVec2 & point) const {
	size_t row;
	size_t column;

	Index(point, row, column);
	if (row == INVALID_INDEX || column == INVALID_INDEX)
		return INVALID_CELL;
	
	return cells[row][column];		// FIXME: test that this is a const reference
}

//******************
// CellWidth
//******************
template< class type, size_t rows, size_t columns>
inline size_t SpatialIndexGrid<type, rows, columns>::CellWidth() const {
	return cellWidth;
}

//******************
// CellHeight
//******************
template< class type, size_t rows, size_t columns>
inline size_t SpatialIndexGrid<type, rows, columns>::CellHeight() const {
	return cellHeight;
}

//******************
// SetCellWidth
//******************
template< class type, size_t rows, size_t columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellWidth(const size_t cellWidth) {
	this->cellWidth = cellWidth;
}

//******************
// SetCellHeight
//******************
template< class type, size_t rows, size_t columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellHeight(const size_t cellHeight) {
	this->cellHeight = cellHeight;
}

#endif /* EVIL_SPATIAL_INDEX_GRID_H */

