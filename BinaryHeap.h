#ifndef EVIL_BINARY_HEAP_H
#define EVIL_BINARY_HEAP_H

#include <new.h>		// std::move
#include <utility>		// std::swap

// TODO (A* search): use lazy deletion for IncreaseKey/DecreaseKey: insert the new data at the bottom and heapify,
// AND somehow mark the old data as "visited" then just pop it when it reaches the top

// TODO: two priority queues of eTiles, each eTile will have:
// A float for its weight (for A* search) (one way of ordering a heap)
// An int for its z-depth (for draw order heapsort) (another way of ordering, then sorting, a heap)

// TODO: generalize things to be drawn in renderEntity_t structs such that a tile has one and an entity has one, and when
// they're ready to be drawn they 

// TODO (draw order): only create a heap (and heapsort) renderEntity_ts ready to be drawn and that are visible in
// on the backbuffer (ie nothing should change about them once ready to draw [no increase/decreasekey]).
// Further, this heap should be cleared (not the memory freed) each frame

// TODO (A* search): heap can be cleared once the search is complete (not the memory freed)
// AND after its list of best waypoints has been pushed onto a bestPath deque

//********************************************
//				eHeap
// The template compare function a user creates 
// dictates if this is a MaxHeap or a MinHeap.
// Does not allocate until a new element is added.
// This copies/moves pushed data, so changing 
// the source data will not affect this heap.
// This class only works with contiguous memory.
// User must ensure heap isn't empty via IsEmpty()
// prior to operator[] and PeekRoot()
//*******************************************
template <class type, class lambdaCompare>
class eHeap {
public:

							eHeap() = delete;															// disallow instantiation without lambdaCompare object		
							eHeap(lambdaCompare & compare, const int initialCapacity = defaultCapacity);// relaced default constructor
							eHeap(lambdaCompare & compare, const type * data, const int numElements);	// heapify copy constructor 
							eHeap(lambdaCompare & compare, type * data, const int numElements);			// heapify move constructor 
							eHeap(const eHeap & other);													// copy constructor
							eHeap(eHeap && other) noexcept;												// move constructor
							~eHeap();																	// destructor

	eHeap					operator+(const eHeap & other);			// copy merge
	eHeap					operator+(eHeap && other);				// move merge
	eHeap &					operator+=(const eHeap & other);		// copy meld
	eHeap &					operator+=(eHeap && other);				// move meld
	eHeap &					operator=(eHeap other) noexcept;		// copy and swap assignement
	const type &			operator[](int index) const;			// random access without bounds checking

	const type &			PeekRoot() const;
	void					PushHeap(const type & data);
	void					PushHeap(type && data);
	void					PopRoot() noexcept;
	void					ReplaceRoot(const type & data);
	void					ReplaceRoot(type && data);

	void					Remove(const int index) noexcept;
	void					Replace(const int index, type & newData);
	void					Replace(const int index, type && newData);
 
	int						Size() const;
	bool					IsEmpty() const;

	// memory management
	size_t					Allocated() const;
	void					SetGranularity(const int newGranularity);
	void					Free();
	void					Resize(const int newCapacity);
	void					Clear();
	void					ClearAndResize(const int newCapacity);

private:
	
	void					Allocate(const int newCapacity);
	void					Heapify();
	void					SiftUp(const int index) noexcept;
	void					SiftDown(const int index) noexcept;

private:

	type *					heap;			
	lambdaCompare			compare;
	int						numElements;
	int						granularity;
	int						capacity;

	static const int		defaultCapacity = 1024;
	static const int		defaultGranularity = 1024;
};

//***************
// eHeap::eHeap
// default constructor
//*****************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::eHeap(lambdaCompare & compare, const int initialCapacity) 
	: compare(compare),
	  numElements(0),
	  granularity(defaultGranularity),
	  capacity(initialCapacity),
	  heap(nullptr) 
{
}

//***************
// eHeap::eHeap
// heapify copy constructor
// only works on contiguous memory
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::eHeap(lambdaCompare & compare, const type * data, const int numElements) 
	: compare(compare),
	  numElements(numElements),
	  granularity(defaultGranularity)
{
	int mod;
	int i;

	mod = numElements % granularity;
	if (numElements > 0 && !mod) {
		capacity = numElements;
	} else {
		capacity = numElements + granularity - mod;
	}

	if (data == nullptr) {
		heap = nullptr;
		return;
	}
	heap = new type[capacity];
	for (i = 0; i < numElements; i++)
		heap[i] = data[i];

	Heapify();
}

//***************
// eHeap::eHeap
// heapify move constructor
// only works on contiguous memory
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::eHeap(lambdaCompare & compare, type * data, const int numElements)
	: compare(compare),
	numElements(numElements),
	granularity(defaultGranularity)
{
	int mod;
	int i;

	mod = numElements % granularity;
	if (numElements > 0 && !mod) {
		capacity = numElements;
	} else {
		capacity = numElements + granularity - mod;
	}

	if (data == nullptr) {
		heap = nullptr;
		return;
	}
	heap = new type[capacity];
	for (i = 0; i < numElements; i++)
		heap[i] = std::move(data[i]);

	Heapify();
}

//***************
// eHeap::eHeap
// copy constructor
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::eHeap(const eHeap<type, lambdaCompare> & other) 
	: compare(other.compare),
	  numElements(other.numElements),
	  granularity(other.granularity),
	  capacity(other.capacity)
{
	int i;
	heap = new type[capacity];
	for (i = 0; i < capacity; i++)
		heap[i] = other.heap[i];
}

//***************
// eHeap::eHeap
// move constructor
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::eHeap(eHeap<type, lambdaCompare> && other) noexcept
	: compare(other.compare),
	  numElements(0),
	  granularity(defaultGranularity),
	  capacity(defaultCapacity),
	  heap(nullptr) 
{
	std::swap(numElements, other.numElements);
	std::swap(granularity, other.granularity);
	std::swap(capacity, other.capacity);
	std::swap(heap, other.heap);
}

//***************
// eHeap::~eHeap
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare>::~eHeap() {
	Free();
}

//***************
// eHeap::operator+
// merge, copies originals
// DEBUG: cannot be const function because the invoked heapify copy constructor is not const
//****************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare> eHeap<type, lambdaCompare>::operator+(const eHeap<type, lambdaCompare> & other) {
	int sumElements; 
	int i, j;
	type * newData; 

	sumElements = numElements + other.numElements;
	if (!sumElements)
		return eHeap<type, decltype(compare)>(compare);

	newData = new type[sumElements];
	if (heap != nullptr) {
		for (i = 0; i < numElements; i++)
			newData[i] = heap[i];
	}
	if (other.heap != nullptr) {
		for (/*i == numElements,*/ j = 0; j < other.numElements; i++, j++)
			newData[i] = other.heap[j];
	}

	return eHeap<type, lambdaCompare>(compare, newData, sumElements);
}

//***************
// eHeap::operator+
// merge, moves from originals
// DEBUG: cannot be const function because the invoked heapify move constructor is not const
//****************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare> eHeap<type, lambdaCompare>::operator+(eHeap<type, lambdaCompare> && other) {
	int sumElements;
	int i, j;
	type * newData;

	sumElements = numElements + other.numElements;
	if (!sumElements)
		return eHeap<type, decltype(compare)>(compare);

	newData = new type[sumElements];
	if (heap != nullptr) {
		for (i = 0; i < numElements; i++)
			newData[i] = std::move(heap[i]);
	}
	if (other.heap != nullptr) {
		for (/*i == numElements,*/ j = 0; j < other.numElements; i++, j++)
			newData[i] = std::move(other.heap[j]);
	}

	return eHeap<type, lambdaCompare>(compare, newData, sumElements);
}

//***************
// eHeap::operator+=
// meld, moves from source
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare> & eHeap<type, lambdaCompare>::operator+=(eHeap<type, lambdaCompare> && other) {
	int i;

	if (other.heap == nullptr)
		return *this;

	if (heap == nullptr) {
		Allocate(other.capacity);
	} else if (numElements + other.numElements > capacity) {
		Resize(capacity + other.numElements);
	}

	for (i = 0; i < other.numElements; i++)
		heap[i + numElements] = std::move(other.heap[i]);

	numElements += other.numElements;
	Heapify();
	return *this;
}

//***************
// eHeap::operator+=
// meld, preserves source
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare> & eHeap<type, lambdaCompare>::operator+=(const eHeap<type, lambdaCompare> & other) {
	int i;

	if (other.heap == nullptr)
		return *this;

	if (heap == nullptr) {
		Allocate(other.capacity);
	} else if (numElements + other.numElements > capacity) {
		Resize(capacity + other.numElements);
	}

	for (i = 0; i < other.numElements; i++)
		heap[i + numElements] = other.heap[i];

	numElements += other.numElements;
	Heapify();
	return *this;
}

//***************
// eHeap::operator=
// copy and swap assignement
//***************
template<class type, class lambdaCompare>
inline eHeap<type, lambdaCompare> & eHeap<type, lambdaCompare>::operator=(eHeap<type, lambdaCompare> other) noexcept {
	std::swap(numElements, other.numElements);
	std::swap(granularity, other.granularity);
	std::swap(capacity, other.capacity);
	std::swap(heap, other.heap);
	// DEBUG: lambdaCompare closure object swap is omitted because both eHeaps already contain relevant instances,
	// as well as to avoid invoking deleted default lambda constructor
	return *this;
}

//***************
// eHeap::operator[]
//***************
template<class type, class lambdaCompare>
inline const type & eHeap<type, lambdaCompare>::operator[](int index) const {
	return heap[index];
}

//***************
// eHeap::PeekRoot
//***************
template<class type, class lambdaCompare>
inline const type & eHeap<type, lambdaCompare>::PeekRoot() const {
	return heap[0];
}

//***************
// eHeap::PushHeap
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::PushHeap(const type & data) {
	if (heap == nullptr) {
		Allocate(capacity);
	} else if (numElements + 1 >= capacity) {
		Resize(capacity + 1);
	}
	heap[numElements] = data;
	SiftUp(numElements);
	numElements++;
}

//***************
// eHeap::PeekRoot
// emplace and move
// assumes the given type has a move assignment operator
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::PushHeap(type && data) {
	if (heap == nullptr) {
		Allocate(capacity);
	} else if (numElements + 1 >= capacity) {
		Resize(capacity + 1);
	}
	heap[numElements] = std::move(data);
	SiftUp(numElements);
	numElements++;
}

//***************
// eHeap::PopRoot
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::PopRoot() noexcept {
	if (heap == nullptr) {
		return;
	} else if (numElements == 1) {
		Clear();
		return;
	}
	numElements--;
	std::swap(heap[0], heap[numElements]);
	SiftDown(0);
}

//***************
// eHeap::ReplaceRoot
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::ReplaceRoot(const type & data) {
	if (heap == nullptr) {
		PushHeap(data);
		return;
	} 
	heap[0] = data;
	SiftDown(0);
}

//***************
// eHeap::ReplaceRoot
// emplace and move
// assumes the given type has a proper move assignment operator
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::ReplaceRoot(type && data) {
	if (heap == nullptr) {
		PushHeap(std::move(data));
		return;
	}
	heap[0] = std::move(data);
	SiftDown(0);
}

//***************
// eHeap::Remove
// DEBUG: assert( index >= 0 && index <= numElements)
//****************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Remove(const int index) noexcept {
	numElements--;
	std::swap(heap[index], heap[numElements]);
	SiftDown(index);
}

//***************
// eHeap::Replace
// DEBUG: instead of explicit IncreaseKey and DecreaseKey
// DEBUG: assert( index >= 0 && index <= numElements)
//***************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Replace(const int index, type & newData) {
	heap[index] = newData;
	if (index > 0 && compare(heap[(index - 1) / 2], heap[index])) {		// compare to parent
		SiftUp(index);
	} else {
		SiftDown(index);
	}
}

//***************
// eHeap::Replace
// emplace and move
// DEBUG: instead of explicit IncreaseKey and DecreaseKey
// DEBUG: assert( index >= 0 && index <= numElements)
//****************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Replace(const int index, type && newData) {
	heap[index] = std::move(newData);
	if (index > 0 && compare(heap[(index - 1) / 2], heap[index])) {		// compare to parent
		SiftUp(index);
	} else {
		SiftDown(index);
	}
}

//***************
// eHeap::Size
// returns number of elements in the heap
//***************
template<class type, class lambdaCompare>
inline int eHeap<type, lambdaCompare>::Size() const {
	return numElements;
}

//***************
// eHeap::IsEmpty
// returns true for numElements == 0
//***************
template<class type, class lambdaCompare>
inline bool eHeap<type, lambdaCompare>::IsEmpty() const {
	return numElements == 0;
}

//***************
// eHeap::Heapify
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Heapify() {
	int i;

	// DEBUG: start at the element just above the last leaf child and work upwards
	for (i = numElements / 2; i > 0; i--)
		SiftDown(i - 1);
}

//***************
// eHeap::SiftUp
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::SiftUp(const int index) noexcept {
	int parent;
	int child;

	for (child = index; child > 0; child = parent) {
		parent = (child - 1) / 2;
		if (!compare(heap[parent], heap[child]))
			return;

		std::swap(heap[parent], heap[child]);
	}
}

//***************
// eHeap::SiftDown
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::SiftDown(const int index) noexcept {
	int parent;
	int child;

	// DEBUG: a right child's existance (child + 1) implies that a left child exists, but not vis versa
	for (parent = index, child = 2 * parent + 1; child < numElements; child = 2 * parent + 1) {
		if (child + 1 < numElements && compare(heap[child], heap[child + 1]))
			child++;
		if (!compare(heap[parent], heap[child]))
			return;

		std::swap(heap[parent], heap[child]);
		parent = child;
	}
}

//***************
// eHeap::Allocate
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Allocate(const int newCapacity) {
	Free();
	capacity = newCapacity;
	heap = new type[capacity];
	memset(heap, 0, capacity * sizeof(type));
}

//***************
// eHeap::Allocated
// returns the total memory allocated 
//**************
template<class type, class lambdaCompare>
inline size_t eHeap<type, lambdaCompare>::Allocated() const {
	return (heap == nullptr) * capacity * sizeof(type);
}

//***************
// eHeap::SetGranularity
// adjust the amount by which the heap will
// expand when it runs out of memory
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::SetGranularity(const int newGranularity) {
	granularity = newGranularity;
}

//***************
// eHeap::Free
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Free() {
	if (heap != nullptr) {
		delete[] heap;
		heap = nullptr;
	}
	numElements = 0;
}

//***************
// eHeap::Resize
// increase heap size
// and expand available memory
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Resize(const int newCapacity) {
	type * oldHeap;
	int mod;
	int newCap;
	int i;

	if (newCapacity <= capacity)
		return;

	mod = newCapacity % granularity;
	if (!mod)
		newCap = newCapacity;
	else
		newCap = newCapacity + granularity - mod;

	if (heap == nullptr) {
		capacity = newCap;
		return;
	}

	oldHeap = heap;
	heap = new type[newCap];
	for (i = 0; i < capacity; i++)
		heap[i] = std::move(oldHeap[i]);

	delete[] oldHeap;
	capacity = newCap;
}

//***************
// eHeap::Clear
// reset numElements retain allocated memory to be overwritten
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::Clear() {
	// DEBUG: memsetting the contents of a 
	// std::unique_ptr or std::string will cause a leak
	// because its destructor cannot deallocate its original pointer
//	if (heap != nullptr) {
//		memset(heap, 0, capacity * sizeof(type));
		numElements = 0;
//	}
}

//***************
// eHeap::ClearAndResize
// reset all data and free allocated memory
// set the new heap size to be allocated when the next element is added
//**************
template<class type, class lambdaCompare>
inline void eHeap<type, lambdaCompare>::ClearAndResize(const int newCapacity) {
	Free();
	capacity = newCapacity;
}

#endif /* EVIL_BINARY_HEAP_H */