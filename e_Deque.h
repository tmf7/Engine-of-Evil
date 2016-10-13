#ifndef E_DEQUE_H
#define E_DEQUE_H

// FIXME: this should not be a #define
#define DEFAULT_DEQUE_SIZE 50

// TODO: add a dynamic memory constructor (copy constructor, destructor, and copy assignment) to allow for larger runtime deques
// eg EvilDeque<type>::EvilDeque<type>(size_t capacity) : capacity(capacity) { EvilDeque(); }
// and std::unique_ptr<Node> nodePool; nodePool = new Node[capacity]; (to allocate a contiguous block of memory?)
// and delete[] nodePool;

// uses pre-allocated stack memory to manage pointers
// pushing past the capacity will overwrite the back of the deque
// user code must check if deque is empty before accessing data
template <class type>
class EvilDeque
{
private:

	typedef struct Node_s {
		Node_s *	prev = this;
		Node_s *	next = this;
		type		data;
//			Node_s() : prev(this), next(this), data() {}
	} Node_t;

	int					numElements;
	int					activeSlot;
	Node_t *			front;
	Node_t *			back;
	Node_t				nodePool[DEFAULT_DEQUE_SIZE];

public:

						EvilDeque();
	void				PushFront(const type & data);
//	void				PushBack(const type & data);		// not currently utilized, requres further checks
	bool				PopFront();
	bool				PopBack();

	const type &		Back() const;
	const type &		Front() const;
	const type &		FromFront(int index) const;
	const type &		FromBack(int index) const;

	void				Clear();				// empty the deque of all nodes
	int					Size() const;			// return then number of elements in the deque
	int					Capacity() const;		// return the maximum number of element the deque can contain at any time
	bool				IsEmpty() const;
};

template< class type >
inline EvilDeque<type>::EvilDeque() {
	numElements = 0;
	activeSlot = 0;
	front = nullptr;
	back = nullptr;
}

template< class type >
inline void EvilDeque<type>::PushFront(const type & data) {
	Node_t * newFront;

	newFront = &nodePool[activeSlot++];
	if (activeSlot >= DEFAULT_DEQUE_SIZE)
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

// template< class type >
// inline void EvilDeque<type>::PushBack(const type & data) {
// }

template< class type >
inline bool EvilDeque<type>::PopFront() {
	Node_t * newFront;
	
	if (front == nullptr) {								// empty deque
		return false;
	} else if (front->prev == front) {					// last node in the deque
//		memset(&(front->data), 0, sizeof(front->data));	// ensure no carry-over if data is overwritten
		front = nullptr;
		back = nullptr;
	} else {											// more than one node in the deque
		newFront = front->prev;
		front->prev->next = newFront;
//		front->next = front;
		front->prev = front;
//		memset(&(front->data), 0, sizeof(front->data));	// ensure no carry-over if data is overwritten
		front = newFront;
	}
	numElements--;

	activeSlot--;
	if (activeSlot < 0)
		activeSlot = 0;

	return true;
}

template< class type >
inline bool EvilDeque<type>::PopBack() {
	Node_t * newBack;

	if (back == nullptr) {								// empty deque
		return false;
	} else if (back->next == back) {					// last node in the deque
//		memset(&(back->data), 0, sizeof(back->data));	// ensure no carry-over if data is overwritten
		front = nullptr;
		back = nullptr;
	} else {											// more than one node in the deque
		newBack = back->next;
		back->next->prev = newBack;
		back->next = back;
//		back->prev = back;
//		memset(&(back->data), 0, sizeof(back->data));	// ensure no carry-over if data is overwritten
		back = newBack;
	}
	numElements--;
	// dont decrease the activeSlot
	return true;
}

template< class type >
inline const type & EvilDeque<type>::Front() const {
	return front->data;
}

template< class type >
inline const type & EvilDeque<type>::Back() const {
	return back->data;
}

// returns the data at the node "index" nodes behind the front
template< class type >
inline const type & EvilDeque<type>::FromFront(int index) const {
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

// returns the data at the node "index" nodes ahead of the back
template< class type >
inline const type & EvilDeque<type>::FromBack(int index) const {
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

template< class type >
inline void EvilDeque<type>::Clear() {
	while (PopFront())
		;
}

template< class type >
inline int EvilDeque<type>::Size() const {
	return numElements;
}

// TODO: after implementing the dynamic memory version of deque
// change this to return the "capacity" variable set at construction
template< class type >
inline int EvilDeque<type>::Capacity() const {
	return DEFAULT_DEQUE_SIZE;
}

template< class type >
inline bool EvilDeque<type>::IsEmpty() const {
	return numElements == 0;
}

#endif /* E_DEQUE_H */