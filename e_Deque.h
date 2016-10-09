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
template <class type>
class EvilDeque
{
private:

	struct Node {
			Node *	prev;
			Node *	next;
			type	data;
			Node() : prev(this), next(this), data() {}
	};

	size_t numElements;
	size_t activeSlot;
	Node * front;
	Node * back;
	Node nodePool[DEFAULT_DEQUE_SIZE];

public:

						EvilDeque();
	void				PushFront(const type & data);
//	void				PushBack(const type & data);		// not currently utilized, requres further checks
	bool				PopFront();
	bool				PopBack();

	type *				Back() const;
	type *				Front() const;
	type *				FromFront(const size_t index) const;
	type *				FromBack(const size_t index) const;

	void				Clear();				// empty the stack of all nodes
	size_t				GetSize() const;		// return then number of elements in the stack
	size_t				GetCapacity() const;	// return the maximum number of element the stack can contain at any time
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
	Node * newFront;

	if (activeSlot == DEFAULT_DEQUE_SIZE)
		activeSlot = 0;

	newFront = &nodePool[activeSlot];

	if (newFront == back)
		PopBack();			// overwrite the oldest node in the pre-allocated memory array

	newFront->data = data;

	if (front == nullptr) {
		front = back = newFront;
	} else {
		newFront->prev = front;
		front->next = newFront;
		front = newFront;
	}
	numElements++;
	activeSlot++;
}

// template< class type >
// inline void EvilDeque<type>::PushBack(const type & data) {
// }

template< class type >
inline bool EvilDeque<type>::PopFront() {
	Node * newFront;
	
	if (front == nullptr) {				// empty deque

		return false;
		
	} else if (front->prev == front) {	// last node in the deque

		// front->data = NULL;			// don't waste time clearing the data
		front = back = nullptr;
		return true;

	} else {							// more than one node in the deque

		newFront = front->prev;
		front->next = front;			// redundant?
		front->prev = front;
		// front->data = NULL;			// don't waste time clearing the data
		front = newFront;
		return true;

	}
	numElements--;
	activeSlot--;
}

template< class type >
inline bool EvilDeque<type>::PopBack() {
	Node * newBack;

	if (back == nullptr) {				// empty deque

		return false;

	}
	else if (back->next == back) {		// last node in the deque

		// back->data = NULL;			// don't waste time clearing the data
		front = back = nullptr;
		return true;

	}
	else {								// more than one node in the deque

		newBack = back->next;
		back->next = back;
		back->prev = back;				// redundant?
		// back->data = NULL;			// don't waste time clearing the data
		back = newBack;
		return true;

	}
	numElements--;
	// dont decrease the activeSlot
}

template< class type >
inline type * EvilDeque<type>::Front() const {
	if (front == nullptr)
		return nullptr;
	return &(front->data);
}

template< class type >
inline type * EvilDeque<type>::Back() const {
	if (back == nullptr)
		return nullptr;
	return &(back->data);
}

// returns the data at the node "index" nodes behind the front
template< class type >
inline type * EvilDeque<type>::FromFront(const size_t index) const {
	Node * node;
	size_t i;

	if (index >= numElements)
		return nullptr;

	node = front;
	if (node == nullptr)
		return nullptr;

	if (index == numElements-1)
		return &(back->data);

	i = 0;
	while (i++ < index)
		node = node->prev;

	return &(node->data);
}

// returns the data at the node "index" nodes ahead of the back
template< class type >
inline type * EvilDeque<type>::FromBack(const size_t index) const {
	Node * node;
	size_t i;

	if (index >= numElements)
		return nullptr;

	node = back;
	if (node == nullptr)
		return nullptr;

	if (index == numElements-1)
		return &(front->data);

	i = 0;
	while (i++ < index)
		node = node->next;

	return &(node->data);
}

template< class type >
inline void EvilDeque<type>::Clear() {
	while (PopFront())
		;
}

template< class type >
inline size_t EvilDeque<type>::GetSize() const {
	return numElements;
}

// TODO: after implementing the dynamic memory version of deque
// change this to return the "capacity" variable set at construction
template< class type >
inline size_t EvilDeque<type>::GetCapacity() const {
	return DEFAULT_DEQUE_SIZE;
}

template< class type >
inline bool EvilDeque<type>::IsEmpty() const {
	return front == nullptr;
}

#endif /* E_DEQUE_H */