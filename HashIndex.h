#ifndef EVIL_HASH_INDEX_H
#define EVIL_HASH_INDEX_H

#include <vector>

//************************************
//			HashIndex
//	Fast hash table for indexes and arrays.
//	uses std::vector<int> for memory management
//  DEBUG: resizing the hash while in use invalidates
//  all the hashMask-dependent keys (no matter the type)
//************************************
class eHashIndex {
public:
					
						eHashIndex();
	explicit			eHashIndex(const int initialHashSize);

	void				Add(const int key, const int index);
	void				Remove(const int key, const int index);
	int					First(const int key) const;
	int					Next(const int index) const;

	void				InsertIndex(const int key, const int index);
	void				RemoveIndex(const int key, const int index);
	void				Clear();
	void				ClearAndResize(const int newHashSize);
	int					NumUniqueKeys() const;
	int					NumDuplicateKeys() const;
	int					GetSpread() const;
	int					GenerateKey(const char * string) const;
	int					GenerateKey(const int value) const;

	// slots allocated by std::vector<int>
	size_t				HashCapacity() const;
	size_t				IndexCapacity() const;

private:

	std::vector<int>	hash;
	std::vector<int>	indexChain;
	int					hashMask;

	static const int	defaultHashSize = 1024;		// must be a power of two, represents the number of available unique keys
	static const int	INVALID_INDEX = -1;
};

//*******************
// eHashIndex::eHashIndex
//*******************
inline eHashIndex::eHashIndex() : eHashIndex(defaultHashSize) {
}

//*******************
// eHashIndex::eHashIndex
//*******************
inline eHashIndex::eHashIndex(const int initialHashSize) {
	hash.reserve(initialHashSize);
	indexChain.reserve(initialHashSize);
	hash.assign(initialHashSize, INVALID_INDEX);
	indexChain.assign(initialHashSize, INVALID_INDEX);
	hashMask = hash.size() - 1;
}

//*******************
// eHashIndex::Add
// add an index to the hash
// --only add unique indexes--
// sets this most recently added index as the First() one to be viewed
// DEBUG: assert (index >= 0)
//*******************
inline void eHashIndex::Add(const int key, const int index) {
	if (index >= indexChain.size()) // DEBUG: std::vector may allocate more than max-signed-int values
		indexChain.resize(index + 1);

	int k = key & hashMask;
	indexChain[index] = hash[k];
	hash[k] = index;
}

//*******************
// eHashIndex::Remove
// remove an index from the hash
// --ensure the input key/index pair exists (even if a duplicate)--
// DEBUG: assert( index >= 0 && index < indexChain.size() )
//*******************
inline void eHashIndex::Remove(const int key, const int index) {
	int k;

	if (hash.empty())
		return;
	
	k = key & hashMask;
	if (hash[k] == index) {
		hash[k] = indexChain[index];
	} else {
		for (int i = hash[k]; i != INVALID_INDEX; i = indexChain[i]) {
			if (indexChain[i] == index) {
				indexChain[i] = indexChain[index];
				break;
			}
		}
	}
	indexChain[index] = INVALID_INDEX;
}

//*******************
// eHashIndex::First
// get the first index from the hash, returns -1 if empty hash entry
//*******************
inline int eHashIndex::First(const int key) const {
	return hash[key & hashMask];
}

//*******************
// eHashIndex::Next
// get the next index from the hash, returns -1 if at the end of the hash chain
// index >= 0 && index < indexSize only
//*******************
inline int eHashIndex::Next(const int index) const {
	return indexChain[index];
}


//*******************
// eHashIndex::Insertindex
// insert an new index into the indexChain and add it to the hash, increasing all indexes >= index
// useful for dynamically-sized array indexing
// --ensure the source array has actually resized by one, making the key/index pair valid--
// DEBUG: assert (index >= 0)
//*******************
inline void eHashIndex::InsertIndex(const int key, const int index) {
	int max = index;
	for (size_t i = 0; i < hash.size(); i++) {
		if (hash[i] >= index) {
			hash[i]++;
			if (hash[i] > max) {
				max = hash[i];
			}
		}
	}
	for (size_t i = 0; i < indexChain.size(); i++) {
		if (indexChain[i] >= index) {
			indexChain[i]++;
			if (indexChain[i] > max) {
				max = indexChain[i];
			}
		}
	}
	if (max >= indexChain.size()) {
		indexChain.resize(max + 1);
	}
	for (int i = max; i > index; i--) {
		indexChain[i] = indexChain[i - 1];
	}
	indexChain[index] = INVALID_INDEX;
	Add(key, index);
}

//*******************
// eHashIndex::RemoveIndex
// remove an index from the indexChain and remove it from the hash, decreasing all indexes >= index
// useful for dynamically-sized array indexing 
// ensure the target index is actually removed from the source array first
// DEBUG: assert ( index >= 0 && index < indexChain.size() )
//*******************
inline void eHashIndex::RemoveIndex(const int key, const int index) {
	Remove(key, index);
	int max = index;
	for (size_t i = 0; i < hash.size(); i++) {
		if (hash[i] >= index) {
			if (hash[i] > max) {
				max = hash[i];
			}
			hash[i]--;
		}
	}
	for (size_t i = 0; i < indexChain.size(); i++) {
		if (indexChain[i] >= index) {
			if (indexChain[i] > max) {
				max = indexChain[i];
			}
			indexChain[i]--;
		}
	}
	for (int i = index; i < max; i++) {
		indexChain[i] = indexChain[i + 1];
	}
	indexChain[max] = INVALID_INDEX;
}

//*******************
// eHashIndex::Clear
// only clears the hash table because clearing the indexChain is not really needed
//*******************
inline void eHashIndex::Clear() {
	hash.assign(hash.capacity(), INVALID_INDEX);
}

//*******************
// eHashIndex::Clear
// only clears the hash table because clearing the indexChain is not really needed
//*******************
inline void eHashIndex::ClearAndResize(const int newHashSize) {
	hash.resize(newHashSize);
	hash.assign(hash.capacity(), INVALID_INDEX);
	hashMask = hash.size() - 1;
}

//*******************
// eHashIndex::NumUniqueKeys
// returns the number of unique keys in use
// DEBUG: these are stored in the primary hash vector
//*******************
inline int eHashIndex::NumUniqueKeys() const {
	int uniqueCount = 0;
	for (size_t i = 0; i < hash.size(); i++) {
		if (hash[i] != -1)
			uniqueCount++;
	}
	return uniqueCount;
}

//*******************
// eHashIndex::NumDuplicateKeys
// returns the current number of indexes with duplicate keys
// DEBUG: these are stored in the indexChain vector
//*******************
inline int eHashIndex::NumDuplicateKeys() const {
	int registeredCount = 0;
	for (size_t i = 0; i < indexChain.size(); i++) {
		if (indexChain[i] != -1)
			registeredCount++;
	}
	return registeredCount;
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

//*******************
// eHashIndex::GenerateKey
//*******************
inline int eHashIndex::GenerateKey(const int value) const {
	return value & hashMask;
}

//*******************
// eHashIndex::HashCapacity
//*******************
inline size_t eHashIndex::HashCapacity() const {
	return hash.capacity();
}

//*******************
// eHashIndex::IndexCapacity
//*******************
inline size_t eHashIndex::IndexCapacity() const {
	return indexChain.capacity();
}

#endif /* EVIL_HASH_INDEX_H */