/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_GRID_INDEX_H
#define EVIL_GRID_INDEX_H

#include "Class.h"

namespace evil {

//*************************************************
//				eGridIndex
//  base class of types expected by eSpatialIndexGrid
//*************************************************
class eGridIndex : public eClass {

	ECLASS_DECLARATION(eGridIndex)

public:

	virtual				   ~eGridIndex() = default;

	void					SetGridPosition(const int row, const int column)	{ gridRow = row; gridColumn = column; }
	int						GridRow() const										{ return gridRow; }
	int						GridColumn() const									{ return gridColumn; }

	virtual	void			Reset()												{ inOpenSet = false; inClosedSet = false; }

public:

	// expidites openSet and closedSet vector searches while systematically traversing the eSpatialIndexGrid to which *this belongs (eg: A* search)
	// DEBUG: always reset these values after use
	bool					inOpenSet	= false;	
	bool					inClosedSet = false;

protected:

	int						gridRow;
	int						gridColumn;

};

}      /* evil */
#endif /* EVIL_GRID_INDEX_H */
