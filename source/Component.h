#ifndef EVIL_COMPONENT_H
#define EVIL_COMPONENT_H

#include "Class.h"

class eGameObject;

//*************************************************
//				eComponent
// base class for all in-game interactive objects
// eGameObjects handle the lifetime of eComponents
// DEBUG: always define and construct derived classes
// with an eGameObject-type owner [eg: eRenderImage::eRenderImage(eGameObject * owner)]
//*************************************************
class eComponent : eClass {
private:

	friend class eGameObject;

public:

	const eGameObject *					Owner() const						{ return owner; }
	eGameObject *						Owner()								{ return owner; }

	virtual int							GetClassType() const override		{ return CLASS_COMPONENT; }

protected:

										eComponent() = default;		// safety-reminder, disallow outside classes from instantiation without an owner

	virtual void						SetOwner(eGameObject * newOwner)	{ owner = newOwner; }

protected:

	eGameObject *						owner = nullptr;			// back-pointer to user managing the lifetime of *this

	// DEBUG: std::shared_ptr creates a loop and artificial memory leak because neither this nor its owner can be destroyed
	// DEBUG: raw pointer is okay as long as *this doesn't outlive its owner 

	// FIXME/BUG: *owner may move in memory because of std::vector resizing
	// SOLTUION(1/3): use observer pattern w/std::weak_ptr (owner as the subject, *this as observer)
	// SOLUTION(2/3): OR, update this->owner whenever ANY of owners ctor/assignments are called [cheaper?]
	// which means creating rule of 5 for everything with an eComponent
	// SOLUTION(3/3): OR, only assign owner once the final *owner no longer moves in memory (eg: during eEntity::Spawn)
};

#endif /* EVIL_COMPONENT_H */

