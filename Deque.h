#ifndef EVIL_DEQUE_H
#define EVIL_DEQUE_H

// TODO: add a dynamic memory constructor (copy constructor, destructor, and copy assignment) to allow for larger runtime deques
// eg Deque<type>::Deque<type>(size_t capacity) : capacity(capacity) { Deque(); }
// and std::unique_ptr<Node> nodePool; nodePool = new Node[capacity]; (to allocate a contiguous block of memory?)
// and delete[] nodePool;

// TODO: Node_s should only have type * data (not: type data)
// or use a rvalue referece move operation
// to increase speed/performance (instead of wasting time copying)

//*************************************************
//
//				Deque
//
//*************************************************
// uses pre-allocated stack memory to manage pointers
// and copies source data into its memory block
// pushing past the capacity will overwrite the back of the deque
// user code must check if deque is empty before accessing data
template <class type, size_t capacity>
class Deque {
private:

	typedef struct Node_s {
		Node_s *	prev = this;
		Node_s *	next = this;
		type		data;
	} Node_t;

	int					numElements;
	int					activeSlot;
	Node_t *			front;
	Node_t *			back;
	Node_t				nodePool[capacity];

public:

						Deque();
	void				PushFront(const type & data);
//	void				PushBack(const type & data);		// not currently utilized, requres further checks
	bool				PopFront();
	bool				PopBack();

	const type &		Back() const;
	const type &		Front() const;
	const type &		FromFront(int index) const;
	const type &		FromBack(int index) const;

	void				Clear();
	int					Size() const;
	int					Capacity() const;
	bool				IsEmpty() const;
};

//******************
// Deque
//******************
template <class type, size_t capacity>
inline Deque<type, capacity>::Deque() : numElements(0), activeSlot(0),
										front(nullptr), back(nullptr) {
}

//******************
// PushFront
// links the data to the front node
//******************
template <class type, size_t capacity>
inline void Deque<type, capacity>::PushFront(const type & data) {
	Node_t * newFront;

	newFront = &nodePool[activeSlot++];
	if (activeSlot >= capacity)
		activeSlot = 0;

	// overwrite the oldest node in the pre-allocated memory array
	if (newFront == back)
		PopBack();
	
	newFront->data = data;
	
	if (front == nullptr) {
		front = newFront;
		back = newFront;
	} else {
		newFront->prev = front;
		front->next = newFront;
		front = newFront;
	}
	numElements++;
}

//******************
// PushBack
// links the data to the back node
//******************
// template <class type, size_t capacity>
// inline void Deque<type, capacity>::PushBack(const type & data) {
// }

//******************
// PopFront
// un-links the front node
// returns false when no nodes exist
//******************
template <class type, size_t capacity>
inline bool Deque<type, capacity>::PopFront() {
	Node_t * newFront;
	
	if (front == nullptr) {								// empty deque
		return false;
	} else if (front->prev == front) {					// last node in the deque
		front = nullptr;
		back = nullptr;
	} else {											// more than one node in the deque
		newFront = front->prev;
		front->prev->next = newFront;
		front->prev = front;
		front = newFront;
	}
	numElements--;

	activeSlot--;
	if (activeSlot < 0)
		activeSlot = 0;

	return true;
}

//******************
// PopBack
// un-links the back node
// returns false when no nodes exist
//******************
template <class type, size_t capacity>
inline bool Deque<type, capacity>::PopBack() {
	Node_t * newBack;

	if (back == nullptr) {								// empty deque
		return false;
	} else if (back->next == back) {					// last node in the deque
		front = nullptr;
		back = nullptr;
	} else {											// more than one node in the deque
		newBack = back->next;
		back->next->prev = newBack;
		back->next = back;
		back = newBack;
	}
	numElements--;
	// dont decrease the activeSlot
	return true;
}

//******************
// Front
//******************
template <class type, size_t capacity>
inline const type & Deque<type, capacity>::Front() const {
	return front->data;
}

//******************
// Back
//******************
template <class type, size_t capacity>
inline const type & Deque<type, capacity>::Back() const {
	return back->data;
}

//******************
// FromFront
// returns the data at the node "index" nodes behind the front
//******************
template <class type, size_t capacity>
inline const type & Deque<type, capacity>::FromFront(int index) const {
	Node_t * node;
	int i;

	if (index >= numElements)
		index = numElements - 1;
	else if (index < 0)
		index = 0;

	node = front;
	if (index == numElements-1)
		return back->data;

	i = 0;
	while (i++ < index)
		node = node->prev;

	return node->data;
}

//******************
// FromBack
// returns the data at the node "index" nodes ahead of the back
//******************
template <class type, size_t capacity>
inline const type & Deque<type, capacity>::FromBack(int index) const {
	Node_t * node;
	int i;

	if (index >= numElements)
		index = numElements - 1;
	else if (index < 0)
		index = 0;

	node = back;
	if (index == numElements-1)
		return front->data;

	i = 0;
	while (i++ < index)
		node = node->next;

	return node->data;
}

//******************
// Clear
// empties the deque of all nodes
// starting at the front
//******************
template <class type, size_t capacity>
inline void Deque<type, capacity>::Clear() {
	while (PopFront())
		;
}

//******************
// Size
// current number of nodes
//******************
template <class type, size_t capacity>
inline int Deque<type, capacity>::Size() const {
	return numElements;
}

//******************
// Capacity
// maximum number of nodes allowed
//******************
template <class type, size_t capacity>
inline int Deque<type, capacity>::Capacity() const {
	return capacity;
}

//******************
// IsEmpty
// returns true at zero nodes
//******************
template <class type, size_t capacity>
inline bool Deque<type, capacity>::IsEmpty() const {
	return numElements == 0;
}

#endif /* EVIL_DEQUE_H */