#include "qa.h"

const char *splitVOneGuide() {
	return
	"1.\tYou should see one button `One` filling the entire left side."
	;
}

uiControl* splitVOne()
{
	uiSplit *split;

	split = uiNewVerticalSplit();
	uiSplitAdd1(split, uiControl(uiNewButton("One")), 1, 1);

	return uiControl(split);
}

const char *splitHOneGuide() {
	return
	"1.\tYou should see one button `One` filling the entire left side."
	;
}

uiControl* splitHOne()
{
	uiSplit *split;

	split = uiNewHorizontalSplit();
	uiSplitAdd1(split, uiControl(uiNewButton("One")), 1, 1);

	return uiControl(split);
}

const char *splitVGuide() {
	return
	"1.\tYou should see two buttons `One` and `Two` stacked on top of each\n"
	"\tother. They should be equal in size.\n"
	"\n"
	"2.\tMove your mouse to the line between the two buttons. You should\n"
	"\tsee your mouse cursor turn into a resize cursor.\n"
	"\n"
	"3.\tPress down your left mouse button and move your cursor up.\n"
	"\tThe button `One` should shrink in size while button `Two` fills in\n"
	"\tthe space. You should be able to shrink button `One` until it fully\n"
	"\tdisappears.\n"
	"\n"
	"4.\tWith your left mouse button still pressed move your cursor down.\n"
	"\tObserve how button `One` grows in size and button `Two` shrinks.\n"
	"\tMove down with your cursor until button `Two` fully disappears."
	;
}

uiControl* splitV()
{
	uiSplit *split;

	split = uiNewVerticalSplit();
	uiSplitAdd1(split, uiControl(uiNewButton("One")), 1, 1);
	uiSplitAdd2(split, uiControl(uiNewButton("Two")), 1, 1);

	return uiControl(split);
}

const char *splitHGuide() {
	return
	"1.\tYou should see two buttons `One` and `Two` places next to each\n"
	"\tother. They should be equal in size.\n"
	"\n"
	"2.\tMove your mouse to the line between the two buttons. You should\n"
	"\tsee your mouse cursor turn into a resize cursor.\n"
	"\n"
	"3.\tPress down your left mouse button and move your cursor left.\n"
	"\tThe button `One` should shrink in size while button `Two` fills in\n"
	"\tthe space. You should be able to shrink button `One` until it fully\n"
	"\tdisappears.\n"
	"\n"
	"4.\tWith your left mouse button still pressed move your cursor right.\n"
	"\tObserve how button `One` grows in size and button `Two` shrinks.\n"
	"\tMove right with your cursor until button `Two` fully disappears."
	;
}

uiControl* splitH()
{
	uiSplit *split;

	split = uiNewHorizontalSplit();
	uiSplitAdd1(split, uiControl(uiNewButton("One")), 1, 1);
	uiSplitAdd2(split, uiControl(uiNewButton("Two")), 1, 1);

	return uiControl(split);
}
