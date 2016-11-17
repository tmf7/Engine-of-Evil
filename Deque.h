#ifndef EVIL_DEQUE_H
#define EVIL_DEQUE_H

#include <new.h>

template<class type>
class eDeque;
//*************************************************
//				eNode
// to be used with friend class eDeque<type>
//*************************************************
template<class type>
class eNode {
	
	friend class eDeque<type>;

public :

					eNode()							 : prev(nullptr), next(nullptr), data() {};				// default constructor
	explicit		eNode(const type & data)		 : prev(nullptr), next(nullptr), data(data) {};			// prevent implicit conversion
					eNode(const eNode<type> & other) : prev(nullptr), next(nullptr), data(other.data) {};	// copy constructor
					// DEBUG: destructor omitted because no heap allocation occurs

	eNode<type> &	operator=(const eNode<type> & other) {													// copy assignment
												  data.~type(); 
												  new (&data) type(other.data); 
												  return *this;};	

	const type &	Data() const				{ return data; };
	type &			Data()						{ return data; };
	eNode<type> *	Prev() const				{ return prev; };
	eNode<type> *	Next() const				{ return next; };

private:

	eNode<type> *	prev;
	eNode<type> *	next;
	type			data;
};

//*************************************************
//				eDeque
// uses heap memory to manage copies of pushed data
// user code must check if deque is empty before accessing data
// TODO: replace the global new memory allocation with
// a custom heap memory block allocator (aligned) that has a templated/settable
// granularity and overridden new operator (to reduce the allocation overhead)
// DEBUG: will crash if it fails any new allocation (eg: PushFront, PushBack, operator=, cctor)
//*************************************************
template <class type>
class eDeque {
public:

						eDeque();									// default constructor
						eDeque(const eDeque<type> & other);			// copy constructor
					   ~eDeque();									// destructor
						
	eDeque<type> &		operator=(const eDeque<type> & other);		// copy assignment

	void				PushFront(const type & data);
	void				PushBack(const type & data);
	void				PopFront();
	void				PopBack();

	eNode<type> *		Back() const;
	eNode<type> *		Front() const;
	eNode<type> * 		FromFront(int index) const;
	eNode<type> * 		FromBack(int index) const;

	void				Clear();
	int					Size() const;
	bool				IsEmpty() const;

private:

	eNode<type> *		front;
	eNode<type> *		back;
	int					nodeCount;
};

//******************
// eDeque::eDeque
// empty eDeque
//******************
template <class type>
inline eDeque<type>::eDeque() : nodeCount(0), front(nullptr), back(nullptr) {
}

//******************
// eDeque::eDeque
//******************
template <class type>
inline eDeque<type>::eDeque(const eDeque<type> & other) {
	eNode<type> * otherIterator;

	for (otherIterator = other.back; otherIterator != nullptr; otherIterator = otherIterator->next)
		PushFront(otherIterator->data);
}

//******************
// eDeque::~eDeque
//******************
template <class type>
inline eDeque<type>::~eDeque() {
	Clear();
}

//******************
// eDeque::operator=
// deep copy back to front
//******************
template <class type>
inline eDeque<type> & eDeque<type>::operator=(const eDeque<type> & other) {
	eNode<type> * thisIterator;
	eNode<type> * otherIterator;
	eNode<type> * newFront;

/*
	// QUESTION: would this be faster than what I do in this function? Or, is heap deallocation/allocation more expensive?
	Clear();
	for (otherIterator = other.back; otherIterator != nullptr; otherIterator = otherIterator->next)
		PushFront(otherIterator->data);
*/	

	// destroy and reconstruct pre-allocated memory if *this already has some
	for (thisIterator	= back,				  otherIterator	 = other.back; 
		 thisIterator  != nullptr	  &&	  otherIterator != nullptr;
		 thisIterator	= thisIterator->next, otherIterator  = otherIterator->next)
		(*thisIterator) = (*otherIterator);		// DEBUG: eNode copy assignemnt
	
	// establish a newFront if *this had more nodes than other
	newFront = nullptr;
	if (thisIterator && otherIterator == nullptr)
		newFront = thisIterator->prev;

	// if *this had more nodes, pop all the nodes beyond other's size, down to newFront
	for (thisIterator = front; thisIterator != newFront; thisIterator = front)
		PopFront();

	// if *this had fewer nodes, push all the data remaining in other onto *this
	for (/*continue moving*/; otherIterator != nullptr; otherIterator = otherIterator->next)
		PushFront(otherIterator->data);

	return *this;
}

//******************
// eDeque::PushFront
// copies the data into a new node and links it to the front
//******************
template <class type>
inline void eDeque<type>::PushFront(const type & data) {
	eNode<type> * newFront;

	newFront = new eNode<type>(data);
	if (front == nullptr) {
		front = newFront;
		back = newFront;
	} else {
		newFront->prev = front;
		front->next = newFront;
		front = newFront;
	}
	nodeCount++;
}

//******************
// eDeque::PushBack
// copies the data into a new node and links it to the back
//******************
template <class type>
inline void eDeque<type>::PushBack(const type & data) {
	eNode<type> * newBack;

	newBack = new eNode<type>(data);
	if (back == nullptr) {
		back = newBack;
		front = newBack;
	} else {
		newBack->next = back;
		back->prev = newBack;
		back = newBack;
	}
	nodeCount++;
}

//******************
// eDeque::PopFront
// unlinks and deletes the front node
//******************
template <class type>
inline void eDeque<type>::PopFront() {
	eNode<type> * newFront;
	eNode<type> * oldFront;
	
	if (front->prev == nullptr) {			// last node in the deque
		delete front;
		front = nullptr;
		back = nullptr;
	} else {								// more than one node in the deque
		oldFront = front;
		newFront = front->prev;
		newFront->next = nullptr;
		front = newFront;
		delete oldFront;
	}
	nodeCount--;
}

//******************
// eDeque::PopBack
// unlinks and deletes the back node
//******************
template <class type>
inline void eDeque<type>::PopBack() {
	eNode<type> * newBack;
	eNode<type> * oldBack;

	if (back->next == nullptr) {			// last node in the deque
		delete back;
		front = nullptr;
		back = nullptr;
	} else {								// more than one node in the deque
		oldBack = back;
		newBack = back->next;
		newBack->prev = nullptr;
		back = newBack;
		delete oldBack;
	}
	nodeCount--;
}

//******************
// eDeque::Front
//******************
template <class type>
inline eNode<type> * eDeque<type>::Front() const {
	return front;
}

//******************
// eDeque::Back
//******************
template <class type>
inline eNode<type> * eDeque<type>::Back() const {
	return back;
}

//******************
// eDeque::FromFront
// returns the node "index" nodes behind the front
// index between [ 0, Size() - 1 ]
// returns nullptr for out-of-bounds index or empty deque
//******************
template <class type>
inline eNode<type> * eDeque<type>::FromFront(int index) const {
	eNode<type> * iterator;
	int i;

	if (index >= nodeCount || index < 0)
		return nullptr;

	if (index == nodeCount - 1)
		return back;

	i = 0;
	iterator = front;
	while (i++ < index)
		iterator = iterator->prev;

	return iterator;
}

//******************
// eDeque::FromBack
// returns the node "index" nodes ahead of the back
// index between [ 0, Size() - 1 ]
// returns nullptr for out-of-bounds index or empty deque
//******************
template <class type>
inline eNode<type> * eDeque<type>::FromBack(int index) const {
	eNode<type> * iterator;
	int i;

	if (index >= nodeCount || index < 0)
		return nullptr;
	
	if (index == nodeCount - 1)
		return front;

	i = 0;
	iterator = back;
	while (i++ < index)
		iterator = iterator->next;

	return iterator;
}

//******************
// eDeque::Clear
// unlinks and deletes all nodes front to back
//******************
template <class type>
inline void eDeque<type>::Clear() {
	while (!IsEmpty())
		PopFront();
}

//******************
// eDeque::Size
// current number of nodes
//******************
template <class type>
inline int eDeque<type>::Size() const {
	return nodeCount;
}

//******************
// eDeque::IsEmpty
// returns true for front == nullptr
//******************
template <class type>
inline bool eDeque<type>::IsEmpty() const {
	return front == nullptr;
}

#endif /* EVIL_DEQUE_H */

/* 
 * BEGIN cctor and assignment test
	// note: trail is a pre-existing eDeque<eVec2> in active use
	eDeque<eVec2> A(trail);						// deque cctor test
	eDeque<eVec2> B;
	B = A;										// deque copy assignment test

	eNode<eVec2> * newData = new eNode<eVec2>(eVec2(10.0f, 10.0f));
	eNode<eVec2> * C = trail.Front();
	C != nullptr ? (*C) = (*newData) : (void)0;	// node copy assignment test
	eNode<eVec2> D(*newData);					// node cctor test only copies source node data, not source's next/prev
	delete newData;								// D is still defined because it made a copy of the newData's data	
 * END cctor and assignment test
 */