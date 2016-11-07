#include "HashIndex.h"

//*******************
// eHashIndex::GetSpread
// returns number in the range [0-100] representing the spread over the hash table
//*******************
int eHashIndex::GetSpread() const {
	int numHashItems[DEFAULT_HASH_SIZE];
	int i, index;
	int totalItems, average;
	int error, e;

	totalItems = 0;
	for (i = 0; i < hashSize; i++) {
		numHashItems[i] = 0;
		for (index = hash[i]; index >= 0; index = indexChain[index]) {
			numHashItems[i]++;
		}
		totalItems += numHashItems[i];
	}
	
	if (totalItems <= 1) // one or none items in hash
		return 100;

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
