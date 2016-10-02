#ifndef E_STACK_H
#define E_STACK_H

/////////////////////////////////////////////////////////////////
// This class allows for stack nodes to be instantiated,
// and the stack "container" to which they belong is 
// manipulated using static member functions.
//
// This class does not allocate dynamic/heap memory, 
// so it does not have a destructor and no explicit size limit.
//
// This class relies on user pre-allocated stack memory, 
// and an outside pointer to the top; hence there is 
// no explicit IsEmpty() function.
/////////////////////////////////////////////////////////////////
template <class type>
class EvilStack
{
private:

	EvilStack<type> *			next;
	type *						data;

public:

								EvilStack();

	static void					Push(type &data, EvilStack<type> &node, EvilStack<type> ** top);
	static bool					Pop(EvilStack<type> ** top);
	static type *				Peek(EvilStack<type> ** top);
	static type *				Next(EvilStack<type> ** node);
	static EvilStack<type> *	NextNode(EvilStack<type> ** node);
	static void					Clear(EvilStack<type> ** top);					// empty the stack of all nodes
	static size_t				GetSize(EvilStack<type> ** top);				// return then number of elements in the stack
};

template< class type >
inline EvilStack<type>::EvilStack() {

	next = this;
	data = nullptr;
}

template< class type >
inline void EvilStack<type>::Push(type &data, EvilStack<type> &node, EvilStack<type> ** top) {
	node.data = &data;

	if (*top == nullptr) {
		*top = &node;
	} else {
		node.next = *top;
		*top = &node;
	}
}

template< class type >
inline bool EvilStack<type>::Pop(EvilStack<type> ** top) {
	EvilStack<type> * newTop;
	
	if (*top == nullptr) {				// empty stack

		return false;

	} else if ((*top)->next == *top) {	// last node in the stack

		newTop = nullptr;
		(*top)->data = nullptr;
		*top = newTop;
		return true;

	} else {							// more than one node in the stack

		newTop = (*top)->next;
		(*top)->next = (*top);
		(*top)->data = nullptr;
		*top = newTop;
		return true;

	}
}

template< class type >
inline type * EvilStack<type>::Peek(EvilStack<type> ** top) {
	return (*top)->data;
}

template< class type >
inline type * EvilStack<type>::Next(EvilStack<type> ** node) {
	if (*node == nullptr || (*node)->next == *node)
		return nullptr;
	return (*node)->next->data;
}

template< class type >
inline EvilStack<type> * EvilStack<type>::NextNode(EvilStack<type> ** node) {
	if (*node == nullptr || (*node)->next == *node)
		return nullptr;

	return (*node)->next;
}


template< class type >
inline void EvilStack<type>::Clear(EvilStack<type> ** top) {
	while (Pop(top))
		;
}

template< class type >
inline size_t EvilStack<type>::GetSize(EvilStack<type> ** top) {
	size_t	numElements;
	EvilStack<type>	* node;
	
	numElements = 0;
	if (*top == nullptr)
		return numElements;

	for (node = *top; node->next != node; node = node->next) {
		num++;
	}

	return numElements;
}

#endif /* E_STACK_H */