#ifndef EVIL_SPATIAL_INDEX_GRID_H
#define EVIL_SPATIAL_INDEX_GRID_H

// TODO: given that in most game maps the full [rows][columns] limit of a
// SpatialIndexGrid won't be used ==> potentially declare an array of eBounds 
// to define a series of 2D arrays within the larger block of memory,
// then declare SpatialIndexGrid<...>::SetGrid( size_t gridNumber );
// to quickly swap between what is being checked/drawn 
// without having to re-initialize the array

class eVec2;

//*************************************************
//				Spatial Index Grid
//  Maps points in 2D space to elements of a 2D array
//  by divinding the space into a regular grid of cells.
//  This class uses stack memory.
//  It is recommended to use a typedef such as
//  typedef SpatialIndexGrid<int, num_rows, num_columns> map_t;
//  to better access static error conditions such as
//  map_t::INVALID_INDEX and map_t::INVALID_CELL
//*************************************************
template< class type, int rows, int columns>
class SpatialIndexGrid {
private:

	type					cells[rows][columns];
	int						cellWidth;
	int						cellHeight;

	// these allow for a smaller chunck of the stack-allocated memory block
	// to be utilized in-game for faster value-checking
	int						rowLimit;							// allows for 0 <= row < rowLimit <= rows
	int						columnLimit;						// allows for 0 <= column < columnLimit <= columns

public:

							SpatialIndexGrid();

	void					Index(const eVec2 & point, int & row, int & column) const;
	type &					Index(const eVec2 & point);
	type					Index(const eVec2 & point) const;
	type &					Index(const int row, const int column);
	type					Index(const int row, const int column) const;

	int						CellWidth() const;
	int						CellHeight() const;
	void					SetCellWidth(const int cellWidth);
	void					SetCellHeight(const int cellHeight);

	int						RowLimit() const;
	int						ColumnLimit() const;
	void					SetRowLimit(const int maxRows);
	void					SetColumnLimit(const int maxColumns);

	int						Width() const;
	int						Height() const;

	void					ClearAllCells();
};


//******************
// SpatialIndexGrid
//******************
template< class type, int rows, int columns>
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid() : cellWidth(1), cellHeight(1),
																	rowLimit(rows), columnLimit(columns) {
}

//******************
// Index
// sets the reference row and column to
// grid-area-scaled values using the given point
// snaps out of bounds points to the closest cell
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point, int & row, int & column)  const {

	row = (int)(point.x / cellWidth);
	column = (int)(point.y / cellHeight);
	
	if (row < 0)
		row = 0;
	else if (row >= rowLimit)
		row = rowLimit - 1;

	if (column < 0)
		column = 0;
	else if (column >= columnLimit)
		column = columnLimit - 1;
}

//******************
// Index
// returns cells[row][column] closest to the given point
// to allow modification of the cell value
// users must ensure inputs are within bounds
//******************
template< class type, int rows, int columns>
inline type & SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) {
	int row;
	int column;

	Index(point, row, column);
	return cells[row][column];
}

//******************
// Index
// returns cells[row][column] closest to the given point
// users must ensure inputs are within bounds
//******************
template< class type, int rows, int columns>
inline type SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) const {
	int row;
	int column;

	Index(point, row, column);
	return cells[row][column];
}

//******************
// Index
// returns cells[row][column]
// to allow modification of the cell value
// users must ensure inputs are within bounds
//******************
template< class type, int rows, int columns>
inline type & SpatialIndexGrid<type, rows, columns>::Index(const int row, const int column) {
	return cells[row][column];
}

//******************
// Index
// returns cells[row][column]
// users must ensure inputs are within bounds
//******************
template< class type, int rows, int columns>
inline type SpatialIndexGrid<type, rows, columns>::Index(const int row, const int column) const {
	return cells[row][column];
}

//******************
// CellWidth
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::CellWidth() const {
	return cellWidth;
}

//******************
// CellHeight
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::CellHeight() const {
	return cellHeight;
}

//******************
// SetCellWidth
// minimum width is 1
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellWidth(const int cellWidth) {
	this->cellWidth = cellWidth > 0 ? cellWidth : 1;
}

//******************
// SetCellHeight
// minimum height is 1
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellHeight(const int cellHeight) {
	this->cellHeight = cellHeight > 0 ? cellHeight : 1;
}

//******************
// RowLimit
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::RowLimit() const {
	return rowLimit;
}

//******************
// ColumnLimit
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::ColumnLimit() const {
	return columnLimit;
}

//******************
// SetRowLimit
// minimum limit is 1
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetRowLimit(const int maxRows) {
	rowLimit = maxRows > 0 ? maxRows : 1;
}

//******************
// SetColumnLimit
// minimum limit is 1
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetColumnLimit(const int maxColumns) {
	columnLimit = maxColumns > 0 ? maxColumns : 1;
}

//******************
// Width
// returns rowLimit * cellWidth
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::Width() const {
	return rowLimit * cellWidth;
}

//******************
// Height
// returns columnLimit * cellHeight
//******************
template< class type, int rows, int columns>
inline int SpatialIndexGrid<type, rows, columns>::Height() const {
	return columnLimit * cellHeight;
}

//******************
// ClearAllCells
// memsets the private 2D cells array to 0-value bytes
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::ClearAllCells() {
	memset(&cells[0][0], 0, rows * columns * sizeof(cells[0][0]));
}

#endif /* EVIL_SPATIAL_INDEX_GRID_H */

