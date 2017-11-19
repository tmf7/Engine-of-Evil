#ifndef EVIL_SPATIAL_INDEX_GRID_H
#define EVIL_SPATIAL_INDEX_GRID_H

#include "Vector.h"
#include "Class.h"

//*************************************************
//				eGridIndex
//  base class of types expected by eSpatialIndexGrid
//*************************************************
class eGridIndex : public eClass {
public:

	virtual				~eGridIndex() = default;

	void				SetGridPosition(const int row, const int column)	{ gridRow = row; gridColumn = column; }
	int					GridRow() const										{ return gridRow; }
	int					GridColumn() const									{ return gridColumn; }

	virtual	void		Reset()												{ inOpenSet = false; inClosedSet = false; }
	virtual int			GetClassType() const override						{ return CLASS_GRIDINDEX; }

public:

	// expidites openSet and closedSet vector searches while systematically traversing the eSpatialIndexGrid to which *this belongs (eg: A* search)
	// DEBUG: always reset these values after use
	bool				inOpenSet	= false;	
	bool				inClosedSet = false;

protected:

	int					gridRow;
	int					gridColumn;

};

//*************************************************
//				eSpatialIndexGrid
//  Maps points in 2D space to elements of a 2D array
//  by dividing the space into an orthographic grid of cells.
//  This class uses stack memory, 
//  and expects an eGridIndex template type (see: eSpatialIndexGrid::ResetAllCells)
//  TODO(~): possibly use std::vector<std::unique_ptr<eGridIndex>>
//	to allocate a smaller contiguous memory footprint at runtime (at the cost of slower loadtime),
//  while still taking advantge of polymorphism
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
	int						NumLayers() const;
	int						LayerDepth(const int layer) const;
	int						LayerFromZPosition(int zPosition) const;
	int						MinZPositionFromLayer(const Uint32 layer) const;
	int						MaxZPositionFromLayer(const Uint32 layer) const;
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

	void					ResetAllCells();

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
	  isoCellWidth(2),
	  isoCellHeight(1),
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
// fills neighbors param with pointers to 0-8 adjacent cells
// DEBUG: reduced the number of logical condition checks (lcc):
// [isCenter] == 2 lcc; [isValid] == 1-4 lcc
// 9[isCenter] + 8[isValid] using nested for loop						= 50 lcc (partially off grid = 26-50 lcc)
// 8[isValid] using range-based loop on array							= 32 lcc (partially off grid = 8-32 lcc)
// 8[bitCheck] + 1[offGrid] + 1[totalOnGrid] + 0/8[isValid] on array	= 9 lcc if out, 10 lcc if in (the usual case), 18-42 lcc if partial-in (a rare case)
//**************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::GetNeighbors(const int row, const int column, std::vector<type *> & neighbors) {
	const int rmo = row - 1;
	const int rpo = row + 1;
	const int cmo = column - 1;
	const int cpo = column + 1;
	const std::array<std::pair<int, int>, 8> testNeighbors = { { { rmo, cmo },		// [0]
																 { rmo, column },	// [1]
																 { rmo, cpo },		// [2]
																 { row, cmo },		// [3]
																 { row, cpo },		// [4]
																 { rpo, cmo },		// [5]
																 { rpo, column },	// [6]
																 { rpo, cpo } } };	// [7]

	const Uint8 neighborhood = (((rmo < 0) * BIT(0)) | ((rmo >= usedRows) * BIT(1)) |
							    ((rpo < 0) * BIT(2)) | ((rpo >= usedRows) * BIT(3)) |
							    ((cmo < 0) * BIT(4)) | ((cmo >= usedColumns) * BIT(5)) |
							    ((cpo < 0) * BIT(6)) | ((cpo >= usedColumns) * BIT(7)));
		
	#define OFF_GRID 0b01100110							// separating axis test, (see: eCollision::GetAreaCells, fn-call avoided so OFF_GRID condition never triggers)
	if (neighborhood & OFF_GRID) return;
	#undef OFF_GRID

	#define PARTIALLY_ON_GRID 0b10011001				// opposite of total overlap conditions
	if (neighborhood & PARTIALLY_ON_GRID) {
		for (auto & neighbor : testNeighbors)
			if (IsValid(neighbor.first, neighbor.second))
				neighbors.emplace_back(&cells[neighbor.first][neighbor.second]);
	} else {											// total overlap of grid, no need to validate indexes
		for (auto & neighbor : testNeighbors)
			neighbors.emplace_back(&cells[neighbor.first][neighbor.second]);
	}
	#undef PARTIALLY_ON_GRID
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
inline int eSpatialIndexGrid<type, rows, columns>::MinZPositionFromLayer(const Uint32 layer) const {
	int minLayerZ = 0;
	for (Uint32 i = 0; i < layer; ++i)
		minLayerZ += (layerDepths[i] + 1);		// DEBUG: +1 to ensure layer depth intervals don't touch
	return minLayerZ;
}

//******************
// eSpatialIndexGrid::MaxZPositionFromLayer
// DEBUG: layer > 0 && layer < layerDepths.size()
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::MaxZPositionFromLayer(const Uint32 layer) const {
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
	ResetAllCells();		
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
	layerDepths.emplace_back(depth);
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
// eSpatialIndexGrid::NumLayers
//******************
template< class type, int rows, int columns>
inline int eSpatialIndexGrid<type, rows, columns>::NumLayers() const {
	return layerDepths.size();
}

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
// eSpatialIndexGrid::ResetAllCells
// calls eGridIndex::Reset on all cells
//******************
template< class type, int rows, int columns>
inline void eSpatialIndexGrid<type, rows, columns>::ResetAllCells() {
	for (auto cell = &cells[0][0]; cell < &cells[rows - 1][columns]; ++cell)
		cell->Reset();
}

#endif /* EVIL_SPATIAL_INDEX_GRID_H */

