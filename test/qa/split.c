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
	uiSplitSetFirst(split, uiControl(uiNewButton("One")));

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
	uiSplitSetFirst(split, uiControl(uiNewButton("One")));

	return uiControl(split);
}

const char *splitVerticalGuide() {
	return
	"1.\tYou should see two buttons `One` and `Two` stacked on top of each\n"
	"\tother. They should be equal in size.\n"
	"\n"
	"2.\tMove your mouse to the line between the two buttons. You should\n"
	"\tsee your mouse cursor turn into a resize cursor.\n"
	"\n"
	"3.\tPress down your left mouse button and move your cursor up.\n"
	"\tThe button `One` should shrink in size while button `Two` fills in\n"
	"\tthe space. You should be able to shrink button `One` to it's minimum\n"
	"\tsize, it should however never fully disappear.\n"
	"\n"
	"4.\tWith your left mouse button still pressed move your cursor down.\n"
	"\tObserve how button `One` grows in size and button `Two` shrinks.\n"
	"\tMove down with your cursor until button `Two` shrinks to it's\n"
	"\tminimum size without every fully disappearing."
	;
}

uiControl* splitVertical()
{
	uiSplit *split;

	split = uiNewVerticalSplit();
	uiSplitSetFirst(split, uiControl(uiNewButton("One")));
	uiSplitSetSecond(split, uiControl(uiNewButton("Two")));

	return uiControl(split);
}

const char *splitHorizontalGuide() {
	return
	"1.\tYou should see two buttons `One` and `Two` places next to each\n"
	"\tother. They should be equal in size.\n"
	"\n"
	"2.\tMove your mouse to the line between the two buttons. You should\n"
	"\tsee your mouse cursor turn into a resize cursor.\n"
	"\n"
	"3.\tPress down your left mouse button and move your cursor left.\n"
	"\tThe button `One` should shrink in size while button `Two` fills in\n"
	"\tthe space. You should be able to shrink button `One` to it's minimum\n"
	"\tsize, it should however never fully disappear.\n"
	"\n"
	"4.\tWith your left mouse button still pressed move your cursor right.\n"
	"\tObserve how button `One` grows in size and button `Two` shrinks.\n"
	"\tMove right with your cursor until button `Two` shrinks to it's\n"
	"\tminimum size without every fully disappearing."
	;
}

uiControl* splitHorizontal()
{
	uiSplit *split;

	split = uiNewHorizontalSplit();
	uiSplitSetFirst(split, uiControl(uiNewButton("One")));
	uiSplitSetSecond(split, uiControl(uiNewButton("Two")));

	return uiControl(split);
}

