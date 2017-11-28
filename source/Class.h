#ifndef EVIL_CLASS_H
#define EVIL_CLASS_H

#include "Definitions.h"

//*************************************************
//					eClass
// base class for all engine of evil classes that 
// can have instantiated objects and are designed 
// to be part of an inheritance chain beyond eClass
//*************************************************
class eClass{
public:

	virtual		   ~eClass() = default;
					eClass() = default;
					eClass(const eClass & other) = default;
					eClass(eClass && other) = default;
	eClass &		operator=(const eClass & other) = default;
	eClass &		operator=(eClass && other) = default;

	virtual	int		GetClassType() const = 0;
};

#endif /* EVIL_CLASS_H */
