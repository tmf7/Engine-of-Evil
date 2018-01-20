/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_HIERARCHY_H
#define EVIL_HIERARCHY_H

#include "ErrorLogger.h"

namespace evil {

//************************************
//			eHierarchy
// Templated node that builds an
// unbalanced tree structure of such nodes.
// owner object memory is managed elsewhere, and
// owners are given a family relationship.
// DEBUG: only uses stack memory
//************************************
template< class type >
class eHierarchy {
public:

							   ~eHierarchy();
	
	void						SetParent( eHierarchy<type> & node );
	void						MakeSiblingAfter( eHierarchy<type> & node );
	bool						ParentedBy( const eHierarchy<type> & node ) const;
	void						RemoveFromParent();
	void						RemoveFromHierarchy();
	void						SetOwner( type * newOwner );
	type *						GetOwner() const;				

	type *						GetParent() const;				
	type *						GetChild() const;				
	type *						GetSibling() const;				
	type *						GetPriorSibling() const;		
	type *						GetNext() const;				
	type *						GetNextLeaf() const;			

private:

	eHierarchy<type> *			GetPriorSiblingNode() const;			

private:

	eHierarchy<type> *			parent	= nullptr;				
	eHierarchy<type> *			sibling = nullptr;
	eHierarchy<type> *			child	= nullptr;
	type *						owner	= nullptr;				

};

//*******************
// eHierarchy::~eHierarchy
//*******************
template< class type >
inline eHierarchy<type>::~eHierarchy() {
	RemoveFromHierarchy();
}

//*******************
// eHierarchy::ParentedBy
// returns true if param node is an ancestor of *this
// returns false otherwise
//*******************
template< class type >
inline bool eHierarchy<type>::ParentedBy( const eHierarchy<type> & node ) const {
	if ( parent == &node ) {
		return true;
	} else if ( parent != nullptr ) {
		return parent->ParentedBy( node );
	}
	return false;
}

//*******************
// eHierarchy::SetParent
// sets *this as the first child of param node
// and param node's first child as the first sibling of *this
//*******************
template< class type >
inline void eHierarchy<type>::SetParent( eHierarchy<type> & node ) {
	RemoveFromParent();
	parent		= &node;
	sibling		= node.child;
	node.child	= this;
}

//*******************
// eHierarchy::MakeSiblingAfter
//*******************
template< class type >
inline void eHierarchy<type>::MakeSiblingAfter( eHierarchy<type> & node ) {
	RemoveFromParent();
	parent = node.parent;
	sibling = node.sibling;
	node.sibling = this;
}

//*******************
// eHierarchy::RemoveFromParent
// makes prior sibling the new child of parent
// then nullifies parent and sibling, but
// retains the child pointer, effectively making
// this a top-level parent node
//*******************
template< class type >
inline void eHierarchy<type>::RemoveFromParent() {
	eHierarchy<type> * prev = GetPriorSiblingNode();

	if ( prev != nullptr ) {
		prev->sibling = sibling;
	} else if ( parent != nullptr ) {
		parent->child = sibling;
	}

	parent = nullptr;
	sibling = nullptr;
}

//*******************
// eHierarchy::RemoveFromHierarchy
// removes *this from the hierarchy
// and adds it's children it's parent, 
// or makes the children's parent nullptr (a top-level set of siblings)
//*******************
template< class type >
inline void eHierarchy<type>::RemoveFromHierarchy() {
	eHierarchy<type> * parentNode	= parent;
	eHierarchy<type> * oldChild		= nullptr;

	RemoveFromParent();
	if ( parentNode != nullptr ) {
		while ( child != nullptr ) {
				oldChild = child;
				oldChild->RemoveFromParent();
				oldChild->SetParent( parentNode );
		}
	} else {
		while ( child != nullptr )
			child->RemoveFromParent();
	}
}

//*******************
// eHierarchy::SetOwner
// sets the object this node is associated with
//*******************
template< class type >
inline void eHierarchy<type>::SetOwner( type * newOwner ) {
	owner = newOwner;
}

//*******************
// eHierarchy::GetOwner
// returns the object associated with this node
//*******************
template< class type >
inline type * eHierarchy<type>::GetOwner() const {
	return owner;
}

//*******************
// eHierarchy::GetParent
// returns the common node among siblings
//*******************
template< class type >
inline type * eHierarchy<type>::GetParent() const {
	return parent->owner;
}

//*******************
// eHierarchy::GetChild
// returns the only child of this node
//*******************
template< class type >
inline type * eHierarchy<type>::GetChild() const {
	return child->owner;
}

//*******************
// eHierarchy::GetSibling
// returns the next node with the same parent
//*******************
template< class type >
inline type * eHierarchy<type>::GetSibling() const {
	return sibling->owner;
}

//*******************
// eHierarchy::GetPriorSibling
// returns the owner of the previous node with the same parent
//*******************
template< class type >
inline type * eHierarchy<type>::GetPriorSibling() const {
	eHierarchy<type> * prev = GetPriorSiblingNode();

	if ( prev != nullptr ) {
		return prev->owner;
	}

	return nullptr;
}

//*******************
// eHierarchy::GetNext
// traverses all nodes of the hierarchy, depth-first
// starting from *this
//*******************
template< class type >
inline type * eHierarchy<type>::GetNext() const {
	if ( child != nullptr ) {
		return child->owner;
	} else if ( sibling != nullptr ) {
		return sibling->owner;
	} else {

		const eHierarchy<type> * parentNode = parent;
		while ( parentNode != nullptr && parentNode->sibling == nullptr ) {
			parentNode = parentNode->parent;
		}

		if ( parentNode != nullptr )
			return parentNode->sibling->owner;
	}

	return nullptr;
}

//*******************
// eHierarchy::GetNextLeaf
// traverses all leaf nodes of the hierarchy
// starting from *this
//*******************
template< class type >
inline type * eHierarchy<type>::GetNextLeaf() const {
	
	// if there is no child or sibling, go up until a parent wth a sibling, then go down its children to a leaf
		
	const eHierarchy<type> *node;

	if ( child != nullptr ) {
		node = child;

		// not a leaf, so go down along child and return the node w/child == nullptr
		while ( node->child != nullptr ) {
			node = node->child;
		}

		return node->owner;
	} else {
		node = this;

		// *this is a leaf, return its neighbor leaf
		while( node != nullptr && node->sibling == nullptr ) {
			node = node->parent;
		}

		if ( node != nullptr ) {
			node = node->sibling;

			// not a leaf, so go down along child and return the node w/child == nullptr
			while ( node->child != nullptr ) {
				node = node->child;
			}

			return node->owner;
		} 
	}

	return nullptr;
}

//*******************
// eHierarchy::GetPriorSiblingNode
// returns previous node with the same parent
// returns nullptr if *this is the first child, or parent is nullptr,
// as well as if *this is not a registered child of its parent
//*******************
template< class type >
inline eHierarchy<type> * eHierarchy<type>::GetPriorSiblingNode() const {
	if ( parent != nullptr ) {
		eHierarchy<type> * prev = parent->child;

		while ( prev != nullptr && prev != this ) {
			if ( prev->sibling == this ) {
				return prev;
			}

			prev = prev->sibling;
		}

		if ( prev == nullptr ) {
			const std::string message( "eHierarchy: node not a registered child of its parent." );
			eErrorLogger::LogError( message.c_str() , __FILE__, __LINE__ );
			eErrorLogger::ErrorPopupWindow( message.c_str() );
		}
	}
	
	return nullptr;
}

}		/* evil */
#endif /* EVIL_HIERARCHY_H */