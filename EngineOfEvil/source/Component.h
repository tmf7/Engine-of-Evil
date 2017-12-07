#ifndef EVIL_COMPONENT_H
#define EVIL_COMPONENT_H

#include "Class.h"

class eGameObject;

//*************************************************
//				eComponent
// base class for all in-game interactive objects
// eGameObjects handle the lifetime of eComponents
//*************************************************
class eComponent : public eClass {
private:

	friend class eGameObject;

public:

	const eGameObject *							Owner() const								{ return owner; }
	eGameObject *								Owner()										{ return owner; }


	virtual int									GetClassType() const override				{ return CLASS_COMPONENT; }
	virtual bool								IsClassType( int classType ) const override	{ 
													if( classType == CLASS_COMPONENT ) 
														return true; 
													return eClass::IsClassType( classType ); 
												}

protected:

												eComponent() = default;		// safety-reminder, disallow outside classes from instantiation without an owner

												// called after owner is constructed, or after it Spawns, depending on user needs
	virtual void								Init()										{}
												
												// called every frame before owner::Think in owner::UpdateComponents
	virtual void								Update()									{}

												// used to maintain the runtime type of a derived eComponent when copying eGameObjects
	virtual std::unique_ptr<eComponent>			GetCopy() const								{ return std::make_unique< eComponent >( *this ); }

	virtual void								SetOwner(eGameObject * newOwner)			{ owner = newOwner; }

protected:

	eGameObject *								owner = nullptr;			// back-pointer to user managing the lifetime of *this
};

#endif /* EVIL_COMPONENT_H */

