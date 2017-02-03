#include "ButtonTypes.h"
#include "Editor.h"

//****************
// eThreeStateButton::Think
// one type of trigger that resets zoom
// TODO: allow the triggered action to be redefined (ie callback?)
// for each eThreeStateButton instance
//****************
void eThreeStateButton::Think() {
	eButton::Think();
	if (IsPressed())
		SetStateEnum(0);
	else if (IsTriggered()) {
		editor.GetCamera().SetZoom(1.0f);
		ClearTrigger();
	}
	else if (IsMouseOver())
		SetStateEnum(2);
	else
		SetStateEnum(1);
};