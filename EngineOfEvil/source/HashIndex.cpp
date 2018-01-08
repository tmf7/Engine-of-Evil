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
#include "HashIndex.h"

using namespace evil;

//*******************
// eHashIndex::GetSpread
// returns an int in the range [0-100] representing the spread over the hash table
//*******************
int eHashIndex::GetSpread() const {
	int i, index, totalItems;
	std::vector<int> numHashItems;
	int average, error, e;

	const int hashSize = hash.size();

	if (NumUniqueKeys() == 0) {
		return 100;
	}

	totalItems = 0;
	numHashItems.reserve(hashSize);
	numHashItems.assign(hashSize, 0);
	for (i = 0; i < hashSize; i++) {
		for (index = hash[i]; index != INVALID_INDEX; index = indexChain[index]) {
			numHashItems[i]++;
		}
		totalItems += numHashItems[i];
	}
	// if no items in hash
	if (totalItems <= 1) {
		return 100;
	}
	average = totalItems / hashSize;
	error = 0;
	for (i = 0; i < hashSize; i++) {
		e = abs(numHashItems[i] - average);
		if (e > 1) {
			error += e - 1;
		}
	}
	return 100 - (error * 100 / totalItems);
}