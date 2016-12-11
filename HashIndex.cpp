#include "HashIndex.h"

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