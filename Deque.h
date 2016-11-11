#ifndef EVIL_DEQUE_H
#define EVIL_DEQUE_H

// TODO: make this an intensive list/node class, instead of an extensive one

//*************************************************
//				eDeque
// uses pre-allocated stack memory to manage pointers
// and copies source data into its memory block
// pushing past the capacity will overwrite the back of the deque
// user code must check if deque is empty before accessing data
//*************************************************

template <class type, size_t capacity>
class eDeque {
private:

	typedef struct Node_s {
		Node_s *	prev = this;
		Node_s *	next = this;
		type 		data;
	} Node_t;

	int					numElements;
	int					activeSlot;
	Node_t *			front;
	Node_t *			back;
	Node_t				nodePool[capacity];			// TODO: have the heap allocator class take care of this instead

public:

						eDeque();

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
// eDeque::eDeque
//******************
template <class type, size_t capacity>
inline eDeque<type, capacity>::eDeque() : numElements(0), activeSlot(0),
										front(nullptr), back(nullptr) {
}

//******************
// eDeque::PushFront
// links the data to the front node
//******************
template <class type, size_t capacity>
inline void eDeque<type, capacity>::PushFront(const type & data) {
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
// eDeque::PushBack
// links the data to the back node
//******************
// template <class type, size_t capacity>
// inline void eDeque<type, capacity>::PushBack(const type & data) {
// }

//******************
// eDeque::PopFront
// un-links the front node
// returns false when no nodes exist
//******************
template <class type, size_t capacity>
inline bool eDeque<type, capacity>::PopFront() {
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
// eDeque::PopBack
// un-links the back node
// returns false when no nodes exist
//******************
template <class type, size_t capacity>
inline bool eDeque<type, capacity>::PopBack() {
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
// eDeque::Front
//******************
template <class type, size_t capacity>
inline const type & eDeque<type, capacity>::Front() const {
	return front->data;
}

//******************
// eDeque::Back
//******************
template <class type, size_t capacity>
inline const type & eDeque<type, capacity>::Back() const {
	return back->data;
}

//******************
// eDeque::FromFront
// returns the data at the node "index" nodes behind the front
//******************
template <class type, size_t capacity>
inline const type & eDeque<type, capacity>::FromFront(int index) const {
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
// eDeque::FromBack
// returns the data at the node "index" nodes ahead of the back
//******************
template <class type, size_t capacity>
inline const type & eDeque<type, capacity>::FromBack(int index) const {
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
// eDeque::Clear
// empties the deque of all nodes
// starting at the front
//******************
template <class type, size_t capacity>
inline void eDeque<type, capacity>::Clear() {
	while (PopFront())
		;
}

//******************
// eDeque::Size
// current number of nodes
//******************
template <class type, size_t capacity>
inline int eDeque<type, capacity>::Size() const {
	return numElements;
}

//******************
// eDeque::Capacity
// maximum number of nodes allowed
//******************
template <class type, size_t capacity>
inline int eDeque<type, capacity>::Capacity() const {
	return capacity;
}

//******************
// eDeque::IsEmpty
// returns true at zero nodes
//******************
template <class type, size_t capacity>
inline bool eDeque<type, capacity>::IsEmpty() const {
	return numElements == 0;
}

#endif /* EVIL_DEQUE_H */