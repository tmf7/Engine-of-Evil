#ifndef EVIL_SPATIAL_INDEX_GRID_H
#define EVIL_SPATIAL_INDEX_GRID_H

class eVec2;

//*************************************************
//				eSpatial Index Grid
//  Maps points in 2D space to elements of a 2D array
//  by dividing the space into a regular grid of cells.
//  This class uses stack memory.
//*************************************************
template< class type, int rows, int columns>
class eSpatialIndexGrid {
public:

							eSpatialIndexGrid();

	bool					IsValid(const int row, const int column) const;
	bool					IsValid(const eVec2 & point) const;
	void					Validate(eVec2 & point) const;
	void					Validate(int & row, int & column) const;

//	void					Index(type * const indexPtr, int & row, int & column) const;
	type &					IndexValidated(const eVec2 & point);
	const type &			IndexValidated(const eVec2 & point) const;
	void					Index(const eVec2 & point, int & row, int & column) const;
	type &					Index(const eVec2 & point);
	const type &			Index(const eVec2 & point) const;
	type &					Index(const int row, const int column);
	const type	&			Index(const int row, const int column) const;

	int						CellWidth() const;
	int						CellHeight() const;
	void					SetCellWidth(const int cellWidth);
	void					SetCellHeight(const int cellHeight);

	// iterator hooks
	type *					begin();
	type *					end();
	const type *			begin() const;
	const type *			end() const;

	int						Rows() const;
	int						Columns() const;
	int						Width() const;
	int						Height() const;

	void					ClearAllCells();

private:

	type					cells[rows][columns];
	int						cellWidth;
	int						cellHeight;
	float					invCellWidth;
	float					invCellHeight;
};

//******************
// eSpatialIndexGrid::eSpatialIndexGrid
//******************
template< class type, int rows, int columns >
inline eSpatialIndexGrid<type, rows, columns>::eSpatialIndexGrid() 
	: cellWidth(1), 
	  cellHeight(1),
	  invCellWidth(1.0f),
	  invCellHeight(1.0f) {
}

//**************
// eSpatialIndexGrid::IsValid
// returns true if point lies within the grid area
//**************
template< class type, int rows, int columns>
inline bool eSpatialIndexGrid<type, rows, columns>::IsValid(const eVec2 & point) const {
	return (point.x >= 0 && point.x <= Width() - 1 && point.y >= 0 && point.y <= Height() - 1);
}

//**************
// eSpatialIndexGrid::IsValid
// returns true if row and column lie within the grid area
//**************
template< class type, int rows, int columns>
inline bool eSpatialIndexGrid<type, rows, columns>::IsValid(const int row, const int column) const {
	return (row >= 0 && row < rows && column >= 0 && column < columns);
}

//******************
// eSpatialIndexGrid::Validate
// snaps points beyond the grid area to the closest in-bounds point
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::Validate(eVec2 & point) const {
	int width;
	int height;

	width = Width() - 1;
	height = Height() - 1;
	if (point.x < 0)
		point.x = 0;
	else if (point.x > width)
		point.x = (float)width;
	
	if (point.y < 0)
		point.y = 0;
	else if (point.y > height)
		point.y = (float)height;
}

//******************
// eSpatialIndexGrid::Validate
// snaps row, column values beyond the bounds of ([0, rows),[0, columns)) to the closest row and/or column
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::Validate(int & row, int & column) const {
	if (row < 0)
		row = 0;
	else if (row >= rows)
		row = rows - 1;

	if (column < 0)
		column = 0;
	else if (column >= columns)
		column = columns - 1;
}

/*
//******************
// eSpatialIndexGrid::Index
// sets the reference row and column using the given pointer into cells, and
// those beyond the bounds of ([0, rows),[0, columns)) to the closest row and/or column
// DEBUG: for possible future use
//******************

template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::Index(type * const indexPtr, int & row, int & column)  const {
	row = (indexPtr - (type *)&cells[0][0]) / columns;
	column = (indexPtr - (type *)&cells[0][0]) % columns;
	Validate(row, column);
}
*/

//******************
// eSpatialIndexGrid::IndexValidated
// returns the valid cell closest to the given point
//******************
template< class type, int rows, int columns>
inline const type & eSpatialIndexGrid<type, rows, columns>::IndexValidated(const eVec2 & point) const {
	int row;
	int column;
	Index(point, row, column);
	Validate(row, column);
	return Index(row, column);
}

//******************
// eSpatialIndexGrid::IndexValidated
// returns the valid cell closest to the given point
//******************
template< class type, int rows, int columns>
inline type & eSpatialIndexGrid<type, rows, columns>::IndexValidated(const eVec2 & point) {
	int row;
	int column;
	Index(point, row, column);
	Validate(row, column);
	return Index(row, column);
}

//******************
// eSpatialIndexGrid::Index
// sets the reference row and column to the cell the point lies within
// user should do eSpatialIndexGrid::Validate(row, column) as needed
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point, int & row, int & column)  const {
	row = (int)(point.x * invCellWidth);		
	column = (int)(point.y * invCellHeight);
}

//******************
// eSpatialIndexGrid::Index
// returns mutable cells[row][column] closest to the given point
//******************
template< class type, int rows, int columns>
inline type & eSpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) {
	int row;
	int column;

	Index(point, row, column);
	return cells[row][column];
}

//******************
// eSpatialIndexGrid::Index
// returns const cells[row][column] closest to the given point
//******************
template< class type, int rows, int columns>
inline const type & eSpatialIndexGrid<type, rows, columns>::Index(const eVec2 & point) const {
	int row;
	int column;

	Index(point, row, column);
	return cells[row][column];
}

//******************
// eSpatialIndexGrid::Index
// returns cells[row][column]
// to allow modification of the cell value
// users must ensure inputs are within bounds of cells[rows][columns]
//******************
template< class type, int rows, int columns>
inline type & eSpatialIndexGrid<type, rows, columns>::Index(const int row, const int column) {
	return cells[row][column];
}

//******************
// eSpatialIndexGrid::Index
// returns cells[row][column]
// users must ensure inputs are within bounds of cells[rows][columns]
//******************
template< class type, int rows, int columns>
inline const type & eSpatialIndexGrid<type, rows, columns>::Index(const int row, const int column) const {
	return cells[row][column];
}

//******************
// eSpatialIndexGrid::CellWidth
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::CellWidth() const {
	return cellWidth;
}

//******************
// eSpatialIndexGrid::CellHeight
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::CellHeight() const {
	return cellHeight;
}

//******************
// eSpatialIndexGrid::SetCellWidth
// minimum width is 1
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::SetCellWidth(const int cellWidth) {
	this->cellWidth = cellWidth > 0 ? cellWidth : 1;
	invCellWidth = 1.0f / (float)this->cellWidth;
}

//******************
// eSpatialIndexGrid::SetCellHeight
// minimum width is 1
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::SetCellHeight(const int cellHeight) {
	this->cellHeight = cellHeight > 0 ? cellHeight : 1;
	invCellHeight = 1.0f / (float)this->cellHeight;
}

//******************
// eSpatialIndexGrid::begin
//******************
template< class type, int rows, int columns>
inline type * eSpatialIndexGrid<type, rows, columns>::begin() {
	return &cells[0][0];
}

//******************
// eSpatialIndexGrid::end
// DEBUG: returns one-past the last element of the contiguous memory block
//******************
template< class type, int rows, int columns>
inline type * eSpatialIndexGrid<type, rows, columns>::end() {
	return &cells[rows - 1][columns];
}

//******************
// eSpatialIndexGrid::begin
//******************
template< class type, int rows, int columns>
inline const type * eSpatialIndexGrid<type, rows, columns>::begin() const {
	return &cells[0][0];
}

//******************
// eSpatialIndexGrid::end
// DEBUG: returns one-past the last element of the contiguous memory block
//******************
template< class type, int rows, int columns>
inline const type * eSpatialIndexGrid<type, rows, columns>::end() const {
	return &cells[rows - 1][columns];
}

//******************
// eSpatialIndexGrid::Rows
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Rows() const {
	return rows;
}

//******************
// eSpatialIndexGrid::Columns
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Columns() const {
	return columns;
}

//******************
// eSpatialIndexGrid::Width
// returns rowLimits * cellWidth
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Width() const {
	return rows * cellWidth;
}

//******************
// eSpatialIndexGrid::Height
// returns columnLimits * cellHeight
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Height() const {
	return columns * cellHeight;
}

//******************
// eSpatialIndexGrid::ClearAllCells
// memsets the private 2D cells array to 0-value bytes
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::ClearAllCells() {
	memset(&cells[0][0], 0, rows * columns * sizeof(cells[0][0]));
}

#endif /* EVIL_SPATIAL_INDEX_GRID_H */

