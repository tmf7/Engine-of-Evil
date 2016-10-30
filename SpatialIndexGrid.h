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

	const int				invalidIndex = -1;					// -1
	type					invalidCell;						// default constructor, changed via SetInvalidCell()

public:

							SpatialIndexGrid();
							SpatialIndexGrid(const int cellWidth, const int cellHeight);
							SpatialIndexGrid(const int cellWidth, const int cellHeight, const int maxRows, const int maxColumns, const type & invalidCell);

	void					Index(const eVec2 & point, int & row, int & column) const;
	type *					Index(const eVec2 & point);
	type *					Index(const int row, const int column);
	const type &			Cell(const int row, const int column) const;
	const type &			Cell(const eVec2 & point) const;

	const int				InvalidIndex() const;
	const type &			InvalidCell() const;
	void					SetInvalidCell(const type & invalidCell);

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
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid() : cellWidth(NULL), cellHeight(NULL),
																	rowLimit(rows), columnLimit(columns) {
}

//******************
// SpatialIndexGrid
//******************
template< class type, int rows, int columns>
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid(const int cellWidth, const int cellHeight) : cellWidth(cellWidth), cellHeight(cellHeight),
																													rowLimit(rows), columnLimit(columns) {
}

//******************
// SpatialIndexGrid
//******************
template< class type, int rows, int columns>
inline SpatialIndexGrid<type, rows, columns>::SpatialIndexGrid(const int cellWidth, const int cellHeight,
																const int maxRows, const int maxColumns,
																const type & invalidCell) : cellWidth(cellWidth), cellHeight(cellHeight),
																							rowLimit(maxRows), columnLimit(maxColumns), INVALID_CELL(invalidCell) {
}

//******************
// Index
// sets the reference row and column to
// grid-area-scaled values using the given point
// users must check for invalid return values
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point, int & row, int & column)  const {

	if (cellWidth == 0 || cellHeight == 0) {
		row = invalidIndex;
		column = invalidIndex;
		return;
	}

	row = (int)(point.x / cellWidth);
	column = (int)(point.y / cellHeight);

	if (row < 0 || row >= rowLimit)
		row = invalidIndex;

	if (column < 0 || column >= columnLimit)
		column = invalidIndex;
}

//******************
// Index
// returns a pointer to cells[r][c] closest to the given point
// to allow modification of the cell value
// users must check for nullptr return value
//******************
template< class type, int rows, int columns>
inline type * SpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) {
	int row;
	int column;

	Index(point, row, column);
	if (row == invalidIndex || column == invalidIndex)
		return nullptr;

	return &cells[row][column];
}

//******************
// Index
// returns a pointer to cells[row][column]
// to allow modification of the cell value
// users must check for nullptr return value
//******************
template< class type, int rows, int columns>
inline type * SpatialIndexGrid<type, rows, columns>::Index(const int row, const int column) {

	if (row < 0 || row >= rowLimit || column < 0 || column >= columnLimit)
		return nullptr;

	return &cells[row][column];
}

//******************
// Cell
// returns cells[row][column] or INVALID_CELL if out of bounds
// users must check for SpatialIndexGrid<type, rows, columns>::INVALID_CELL return value
//******************
template< class type, int rows, int columns>
inline const type & SpatialIndexGrid<type, rows, columns>::Cell(const int row, const int column) const {

	if (row < 0 || row >= rowLimit || column < 0 || column >= columnLimit)
		return invalidCell;

	return cells[row][column];
}

//******************
// Cell
// returns cells[r][c] closest to the given point or INVALID_CELL if out of bounds
// users must check for SpatialIndexGrid<type, rows, columns>::INVALID_CELL return values
//******************
template< class type, int rows, int columns>
inline const type & SpatialIndexGrid<type, rows, columns>::Cell(const eVec2 & point) const {
	int row;
	int column;

	Index(point, row, column);
	if (row == invalidIndex || column == invalidIndex)
		return invalidCell;

	return cells[row][column];
}

//******************
// InvalidIndex
//******************
template< class type, int rows, int columns>
inline const int SpatialIndexGrid<type, rows, columns>::InvalidIndex() const {
	return invalidIndex;
}

//******************
// InvalidCell
//******************
template< class type, int rows, int columns>
inline const type & SpatialIndexGrid<type, rows, columns>::InvalidCell() const {
	return invalidCell;
}

//******************
// SetInvalidCell
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetInvalidCell(const type & invalidCell) {
	this->invalidCell = invalidCell;
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
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellWidth(const int cellWidth) {
	this->cellWidth = cellWidth;
}

//******************
// SetCellHeight
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetCellHeight(const int cellHeight) {
	this->cellHeight = cellHeight;
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
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetRowLimit(const int maxRows) {
	rowLimit = maxRows;
}

//******************
// SetColumnLimit
//******************
template< class type, int rows, int columns>
inline void SpatialIndexGrid<type, rows, columns>::SetColumnLimit(const int maxColumns) {
	columnLimit = maxColumns;
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

