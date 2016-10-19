#ifndef EVIL_DEQUE_H
#define EVIL_DEQUE_H

// TODO: add a dynamic memory constructor (copy constructor, destructor, and copy assignment) to allow for larger runtime deques
// eg Deque<type>::Deque<type>(size_t capacity) : capacity(capacity) { Deque(); }
// and std::unique_ptr<Node> nodePool; nodePool = new Node[capacity]; (to allocate a contiguous block of memory?)
// and delete[] nodePool;


//==================================
//
// Deque
//
//==================================
// uses pre-allocated stack memory to manage pointers
// pushing past the capacity will overwrite the back of the deque
// user code must check if deque is empty before accessing data
template <class type>
class Deque {
private:

	typedef struct Node_s {
		Node_s *	prev = this;
		Node_s *	next = this;
		type		data;
	} Node_t;

	static const int	defaultCapacity = 50;
	int					capacity;
	int					numElements;
	int					activeSlot;
	Node_t *			front;
	Node_t *			back;
	Node_t				nodePool[defaultCapacity];

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
template< class type >
inline Deque<type>::Deque() : numElements(0), activeSlot(0),
										front(nullptr), back(nullptr), capacity(defaultCapacity) {
}

//******************
// PushFront
// links the data to the front node
//******************
template< class type >
inline void Deque<type>::PushFront(const type & data) {
	Node_t * newFront;

	newFront = &nodePool[activeSlot++];
	if (activeSlot >= defaultCapacity)
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
// template< class type >
// inline void Deque<type>::PushBack(const type & data) {
// }

//******************
// PopFront
// un-links the front node
// returns false when no nodes exist
//******************
template< class type >
inline bool Deque<type>::PopFront() {
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
template< class type >
inline bool Deque<type>::PopBack() {
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
template< class type >
inline const type & Deque<type>::Front() const {
	return front->data;
}

//******************
// Back
//******************
template< class type >
inline const type & Deque<type>::Back() const {
	return back->data;
}

//******************
// FromFront
// returns the data at the node "index" nodes behind the front
//******************
template< class type >
inline const type & Deque<type>::FromFront(int index) const {
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
template< class type >
inline const type & Deque<type>::FromBack(int index) const {
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
template< class type >
inline void Deque<type>::Clear() {
	while (PopFront())
		;
}

//******************
// Size
// current number of nodes
//******************
template< class type >
inline int Deque<type>::Size() const {
	return numElements;
}

//******************
// Capacity
// maximum number of nodes allowed
//******************
template< class type >
inline int Deque<type>::Capacity() const {
	return capacity;
}

//******************
// IsEmpty
// returns true at zero nodes
//******************
template< class type >
inline bool Deque<type>::IsEmpty() const {
	return numElements == 0;
}

#endif /* EVIL_DEQUE_H */