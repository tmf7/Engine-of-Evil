#ifndef EVIL_HASH_INDEX_H_
#define EVIL_HASH_INDEX_H_

#include <Windows.h>		// for memset

//************************************
//			HashIndex
//	Fast hash table for indexes and arrays.
//	uses stack-allocated memory
//************************************

#define DEFAULT_HASH_SIZE			1024		// must be a power of two

class eHashIndex {
public:
	eHashIndex();

	void			Add(const int key, const int index);
	void			Remove(const int key, const int index);
	int				First(const int key) const;
	int				Next(const int index) const;

//	void			InsertIndex(const int key, const int index);
//	void			RemoveIndex(const int key, const int index);
	void			Clear();
	int				GetHashSize() const;
	int				GetIndexSize() const;
	int				GetSpread() const;
	int				GenerateKey(const char * string) const;

private:
	int				hashSize;
	int				hashPool[DEFAULT_HASH_SIZE];
	int	*			hash;
	int				indexSize;
	int				indexChain[DEFAULT_HASH_SIZE];
	int				hashMask;
	int				lookupMask;

	static int		INVALID_INDEX[1];

	void			Init();
};

//*******************
// eHashIndex::eHashIndex
//*******************
inline eHashIndex::eHashIndex() {
	hashSize = DEFAULT_HASH_SIZE;
	indexSize = DEFAULT_HASH_SIZE;
	hashMask = hashSize - 1;
	hash = INVALID_INDEX;
	lookupMask = 0;
}

//*******************
// eHashIndex::Init
//*******************
inline void eHashIndex::Init() {
	hash = hashPool;
	memset(hash, 0xff, hashSize * sizeof(hash[0]));
	memset(indexChain, 0xff, indexSize * sizeof(indexChain[0]));
	lookupMask = -1;
}

//*******************
// eHashIndex::Add
// add an index to the hash, assumes the index has not yet been added to the hash
// index >= 0 && index < indexSize only
//*******************
inline void eHashIndex::Add(const int key, const int index) {
	int k;

	if (hash == INVALID_INDEX)
		Init();

	k = key & hashMask;
	indexChain[index] = hash[k];
	hash[k] = index;
}

//*******************
// eHashIndex::Remove
// remove an index from the hash
//*******************
inline void eHashIndex::Remove(const int key, const int index) {
	int k;

	if (hash == INVALID_INDEX)
		return;
	
	k = key & hashMask;
	if (hash[k] == index) {
		hash[k] = indexChain[index];
	}
	else {
		for (int i = hash[k]; i != -1; i = indexChain[i]) {
			if (indexChain[i] == index) {
				indexChain[i] = indexChain[index];
				break;
			}
		}
	}
	indexChain[index] = -1;
}

//*******************
// eHashIndex::First
// get the first index from the hash, returns -1 if empty hash entry
//*******************
inline int eHashIndex::First(const int key) const {
	return hash[key & hashMask & lookupMask];
}

//*******************
// eHashIndex::Next
// get the next index from the hash, returns -1 if at the end of the hash chain
// index >= 0 && index < indexSize only
//*******************
inline int eHashIndex::Next(const int index) const {
	return indexChain[index & lookupMask];
}

/*
//*******************
// eHashIndex::Insertindex
// insert an entry into the index and add it to the hash, increasing all indexes >= index
// index >= 0 && index < indexSize only
//*******************
inline void eHashIndex::InsertIndex(const int key, const int index) {
	int i;
	int max;

	max = index;
	for (i = 0; i < hashSize; i++) {
		if (hash[i] >= index) {
			hash[i]++;
			if (hash[i] > max) {
				max = hash[i];
			}
		}
	}
	for (i = 0; i < indexSize; i++) {
		if (indexChain[i] >= index) {
			indexChain[i]++;
			if (indexChain[i] > max) {
				max = indexChain[i];
			}
		}
	}
	for (i = max; i > index; i--) {
		indexChain[i] = indexChain[i - 1];
	}
	indexChain[index] = -1;
	Add(key, index);
}

//*******************
// eHashIndex::RemoveIndex
// remove an entry from the index and remove it from the hash, decreasing all indexes >= index
//*******************
inline void eHashIndex::RemoveIndex(const int key, const int index) {
	int i;
	int max;

	Remove(key, index);
	max = index;
	for (i = 0; i < hashSize; i++) {
		if (hash[i] >= index) {
			if (hash[i] > max) {
				max = hash[i];
			}
			hash[i]--;
		}
	}
	for (i = 0; i < indexSize; i++) {
		if (indexChain[i] >= index) {
			if (indexChain[i] > max) {
				max = indexChain[i];
			}
			indexChain[i]--;
		}
	}
	for (i = index; i < max; i++) {
		indexChain[i] = indexChain[i + 1];
	}
	indexChain[max] = -1;
}
*/

//*******************
// eHashIndex::Clear
// only clears the hash table because clearing the indexChain is not really needed
//*******************
inline void eHashIndex::Clear() {
	if (hash != INVALID_INDEX)
		memset(hash, 0xff, hashSize * sizeof(hash[0]));
}

//*******************
// eHashIndex::GetHashSize
//*******************
inline int eHashIndex::GetHashSize() const {
	return hashSize;
}

//*******************
// eHashIndex::GetIndexSize
//*******************
inline int eHashIndex::GetIndexSize() const {
	return indexSize;
}

//*******************
// eHashIndex::GenerateKey
// returns a case-sensitive key for a string
//*******************
inline int eHashIndex::GenerateKey(const char * string) const {
	int i;
	int key;

	key = 0;
	for (i = 0; *string != '\0'; i++) 
		key += (*string++) * (i + 119);

	return key;
}

#endif /* EVIL_HASH_INDEX_H_ */


