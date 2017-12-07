#ifndef EVIL_BUTTON_TYPES_H
#define EVIL_BUTTON_TYPES_H

#include "Button.h"

//*********************************
//		eThreeStateButton
//*********************************
class eThreeStateButton : public eButton {
public:
//	void		Init();		// TODO: use this to define a trigger callback
	void		Think();

//	template<class Func, class... Args>
//	void DefineCallback(Func f, Args&&... args) { f(args); };
};
#endif /* EVIL_BUTTON_TYPES_H */
