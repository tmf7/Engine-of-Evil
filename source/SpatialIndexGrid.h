#ifndef EVIL_SPATIAL_INDEX_GRID_H
#define EVIL_SPATIAL_INDEX_GRID_H

#include <vector>
#include "Vector.h"
#include "Class.h"

//*************************************************
//				eSpatial Index Grid
//  Maps points in 2D space to elements of a 2D array
//  by dividing the space into an orthographic grid of cells.
//  This class uses stack memory.
//*************************************************
template< class type, int rows, int columns>
class eSpatialIndexGrid : public eClass {
public:

							eSpatialIndexGrid();
//							eSpatialIndexGrid(const eSpatialIndexGrid & other);
//							eSpatialIndexGrid(eSpatialIndexGrid && other);
//							~eSpatialIndexGrid() = default;

//	eSpatialIndexGrid &		operator=(eSpatialIndexGrid other);

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

	void					GetNeighbors(const int row, const int column, std::vector<type *> & neighbors);

	int						IsometricCellWidth() const;
	int						IsometricCellHeight() const;
	int						LayerDepth(const int layer) const;
	int						LayerFromZPosition(int zPosition) const;
	int						MinZPositionFromLayer(const int layer) const;
	int						MaxZPositionFromLayer(const int layer) const;
	int						CellWidth() const;
	int						CellHeight() const;
	void					SetGridSize(const int numRows, const int numColumns);
	void					SetCellSize(const int cellWidth, const int cellHeight);
	void					AddLayerDepth(const size_t depth);

	// iterator hooks
	// DEBUG: if (usedRows < rows && usedColumns < columns) iteration reads unused cells
//	type *					begin();
//	type *					end();
//	const type *			begin() const;
//	const type *			end() const;

	int						Rows() const;
	int						Columns() const;
	int						Width() const;
	int						Height() const;

	void					ClearAllCells();

	virtual int				GetClassType() const override { return CLASS_SPATIALINDEXGRID; }

private:

	type					cells[rows][columns];
	int						cellWidth;
	int						cellHeight;
	int						usedRows;
	int						usedColumns;
	std::vector<int>		layerDepths;				// 3D world-space increment sets eTile's renderBlock zPos from its layer (and eEntitiy layer from zPos)
	int						isoCellWidth;				// to visually adjust images
	int						isoCellHeight;				// to visually adjust images
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
	  invCellHeight(1.0f),
	  usedRows(rows),
	  usedColumns(columns) {
}
/*
//******************
// eSpatialIndexGrid::eSpatialIndexGrid
//******************
template< class type, int rows, int columns >
inline eSpatialIndexGrid<type, rows, columns>::eSpatialIndexGrid(const eSpatialIndexGrid & other) 
	: cellWidth(other.cellWidth), 
	  cellHeight(other.cellHeight),
	  layerDepth(other.layerDepth),
	  isoCellWidth(other.isoCellWidth),
	  isoCellHeight(other.isoCellHeight),
	  invCellWidth(other.invCellWidth),
	  invCellHeight(other.invCellHeight) {
	std::copy(other.begin(), other.end(), begin());		// DEBUG: shouldn't throw std::bad_alloc as both items exist on the stack
}

//******************
// eSpatialIndexGrid::eSpatialIndexGrid
//******************
template< class type, int rows, int columns >
inline eSpatialIndexGrid<type, rows, columns>::eSpatialIndexGrid(eSpatialIndexGrid && other) 
	: cellWidth(other.cellWidth), 
	  cellHeight(other.cellHeight),
	  layerDepth(other.layerDepth),
	  isoCellWidth(other.isoCellWidth),
	  isoCellHeight(other.isoCellHeight),
	  invCellWidth(other.invCellWidth),
	  invCellHeight(other.invCellHeight) {
	cells = &other.cells[0][0];				// DEBUG: &cells[0][0] == &other.cells[0][0], then other gets destroyed (only freeing other.cells handle...?)
}

//******************
// eSpatialIndexGrid::operator=
//******************
template< class type, int rows, int columns >
inline eSpatialIndexGrid<type, rows, columns> & eSpatialIndexGrid<type, rows, columns>::operator=(eSpatialIndexGrid other) {
	std::swap(cells, other.cells);
	std::swap(cellWidth, other.cellWidth);
	std::swap(cellHeight, other.cellHeight);
	std::swap(layerDepth, other.layerDepth);
	std::swap(isoCellWidth, other.isoCellWidth);
	std::swap(isoCellHeight, other.isoCellHeight);
	std::swap(invCellWidth, other.invCellWidth);
	std::swap(invCellHeight, other.invCellHeight);
	return *this;
}
*/

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
	return (row >= 0 && row < usedRows && column >= 0 && column < usedColumns);
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
	else if (row >= usedRows)
		row = usedRows - 1;

	if (column < 0)
		column = 0;
	else if (column >= usedColumns)
		column = usedColumns - 1;
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
// DEBUG: users must ensure input is within bounds of (rows * cellHeight) * (columns * cellWidth)
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
// DEBUG: users must ensure input is within bounds of (rows * cellHeight) * (columns * cellWidth)
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

//**************
// eSpatialIndexGrid::GetNeighbors
//**************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::GetNeighbors(const int row, const int column, std::vector<type *> & neighbors) {
	for (int r = -1; r <= 1; ++r) {
		for (int c = -1; c <= 1; ++c) {
			if (r == 0 && c == 0)
				continue;

			int checkRow = row + r;
			int checkCol = column + c;
			if (IsValid(checkRow, checkCol))
				neighbors.push_back(&Index(checkRow, checkCol));
		}
	}
}

//******************
// eSpatialIndexGrid::IsometricCellWidth
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::IsometricCellWidth() const {
	return isoCellWidth;
}

//******************
// eSpatialIndexGrid::IsometricCellHeight
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::IsometricCellHeight() const {
	return isoCellHeight;
}

//******************
// eSpatialIndexGrid::LayerDepth
// DEBUG: layer > 0 && layer < layerDepths.size()
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::LayerDepth(const int layer) const {
	return layerDepths[layer];
}

//******************
// eSpatialIndexGrid::MinLayerFromZPosition
// DEBUG: z < 0.0f will return 0, z > highest z will return maximum available layer
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::LayerFromZPosition(int zPosition) const {
	int layer = 0;
	const int maxLayer = layerDepths.size();
	while (layer < maxLayer && (zPosition - layerDepths[layer]) > 0) {
		zPosition -= layerDepths[layer++];
	}
	return layer;
}

//******************
// eSpatialIndexGrid::MinZPositionFromLayer
// DEBUG: layer > 0 && layer < layerDepths.size()
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::MinZPositionFromLayer(const int layer) const {
	int minLayerZ = 0;
	for (int i = 0; i < layer; ++i)
		minLayerZ += (layerDepths[i] + 1);		// DEBUG: +1 to ensure layer depth intervals don't touch
	return minLayerZ;
}

//******************
// eSpatialIndexGrid::MaxZPositionFromLayer
// DEBUG: layer > 0 && layer < layerDepths.size()
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::MaxZPositionFromLayer(const int layer) const {
	return MinLayerZ(layer) + layerDepths[layer];
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
// eSpatialIndexGrid::SetGridSize
// DEBUG: usedRows and usedColumns ranges are [1, rows] and [1, columns], respectively
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::SetGridSize(const int numRows, const int numColumns) {
//	ClearAllCells();		// DEBUG: doing this during eMap::LoadMap causes a read access error during eGridCell::AddTileOwned
	usedRows = numRows > 0 ? (numRows <= rows ? numRows : rows) : 1;
	usedColumns = numColumns > 0 ? (numColumns <= columns ? numColumns : columns) : 1;
}

//******************
// eSpatialIndexGrid::SetCellSize
// DEBUG: minimum width and height are 1
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::SetCellSize(const int cellWidth, const int cellHeight) {
	this->cellWidth = cellWidth > 0 ? cellWidth : 1;
	this->cellHeight = cellHeight > 0 ? cellHeight : 1;
	invCellWidth = 1.0f / (float)this->cellWidth;
	invCellHeight = 1.0f / (float)this->cellHeight;
	isoCellWidth = cellWidth + cellHeight;				// DEBUG: formula results of converting a rectangle's vertices using eMath::CartesianToIsometric
	isoCellHeight = isoCellWidth >> 1;					// DEBUG: same here
}

//******************
// eSpatialIndexGrid::AddLayerDepth
// DEBUG: depth >= 0
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::AddLayerDepth(const size_t depth) {
	layerDepths.push_back(depth);
}

/*
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
*/

//******************
// eSpatialIndexGrid::Rows
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Rows() const {
	return usedRows;
}

//******************
// eSpatialIndexGrid::Columns
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Columns() const {
	return usedColumns;
}

//******************
// eSpatialIndexGrid::Width
// returns rowLimits * cellWidth
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Width() const {
	return usedRows * cellWidth;
}

//******************
// eSpatialIndexGrid::Height
// returns columnLimits * cellHeight
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::Height() const {
	return usedColumns * cellHeight;
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

