#include "uipriv_windows.hpp"

/*
 * TODO block cursor:
 * https://forums.codeguru.com/showthread.php?303994-How-to-disable-mouse-move-event
 */

struct splitChild {
	uiControl *c;
	int stretchy;
	int shrink;
	int collapse;
	int width;
	int height;
};

struct uiSplit {
	uiWindowsControl c;
	HWND hwnd;
	std::vector<struct splitChild> *controls;
	int vertical;
	int padded;
	int moving;
	float ratio;
	int mouseOffset;
	HCURSOR cursor;
};

static void splitPadding(uiSplit *b, int *xpadding, int *ypadding);

static BOOL onSplitter(uiSplit *s, int px, int py)
{
	int xpadding, ypadding;
	splitPadding(s, &xpadding, &ypadding);

	int x = 0;
	int y = 0;
	for (const struct splitChild &sc : *(s->controls)) {
		if (!uiControlVisible(sc.c))
			continue;

		x += sc.width;
		y += sc.height;

		if (s->vertical) {
			if (py >= y && py < y + ypadding)
				return TRUE;
		} else {
			if (px >= x && px < x + xpadding)
				return TRUE;
		}
		x += xpadding;
		y += ypadding;
	}
	return FALSE;
}

static int nControlsVisible(uiSplit *s)
{
	int n = 0;

	for (struct splitChild &sc : *(s->controls)) {
		if (uiControlVisible(sc.c))
			++n;
	}
	return n;
}


static void onResize(uiWindowsControl *c);

// Modified containerWndProc
// TODO add a mechanism to mainly reuse containerWndProc
static LRESULT CALLBACK splitWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	MINMAXINFO *mmi = (MINMAXINFO *) lParam;
	int minwid, minht;
	LRESULT lResult;
	uiSplit *s = uiSplit(dwRefData);

	switch (uMsg) {
		/*
	case WM_CREATE:
		if (s == NULL) {
			s = (uiSplit *) (cs->lpCreateParams);
			s->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) s);
		}
		break;
		*/
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		onResize(uiWindowsControl(s));
		return 0;
	case WM_GETMINMAXINFO:
		lResult = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		uiWindowsControlMinimumSize(uiWindowsControl(s), &minwid, &minht);
		mmi->ptMinTrackSize.x = minwid;
		mmi->ptMinTrackSize.y = minht;
		return lResult;
	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			if (nControlsVisible(s) < 2)
				return 0;

			if (s->vertical)
				s->mouseOffset = y - s->controls->at(0).height;
			else
				s->mouseOffset = x - s->controls->at(0).width;

			printf("mouse offset %d\n", s->mouseOffset);

			s->moving = 1;
			SetCapture(hwnd);
		}
		return 0;
	case WM_LBUTTONUP:
		if (s->moving) {
			s->moving = 0;
			ReleaseCapture();
		}
		return 0;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON && s->moving) {
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			GetClientRect(hwnd, &r);
			if (s->vertical && y - s->mouseOffset > r.bottom)
				return 0;
			if (!s->vertical && x - s->mouseOffset > r.right)
				return 0;
			/*
			if (x > r.right || y > r.bottom)
				return 0;
			*/

			printf("move %d %d\n", x, y);

			if (s->vertical)
				s->ratio = 1.0 * (y - s->mouseOffset) / r.bottom;
			else
				s->ratio = 1.0 * (x - s->mouseOffset) / r.right;

			if (s->ratio < 0.0)
				s->ratio = 0.0;
			if (s->ratio > 1.0)
				s->ratio = 1.0;

			onResize(uiWindowsControl(s));
		}
		return 0;
		/*
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL) {
			logLastError(L"error beginning container paint");
			// bail out; hope DefWindowProc() catches us
			break;
		}
		r = ps.rcPaint;
		paintContainerBackground(hwnd, dc, &r);
		EndPaint(hwnd, &ps);
		return 0;
	// tab controls use this to draw the background of the tab area
	case WM_PRINTCLIENT:
		uiWindowsEnsureGetClientRect(hwnd, &r);
		paintContainerBackground(hwnd, (HDC) wParam, &r);
		return 0;
	case WM_ERASEBKGND:
		// avoid some flicker
		// we draw the whole update area anyway
		return 1;
		*/
	case WM_SETCURSOR:
		{
			POINT pt;
			DWORD pos;

			if (LOWORD(lParam) != HTCLIENT)
				break;
			if (nControlsVisible(s) < 2)
				break;

			pos = GetMessagePos();
			pt.x = GET_X_LPARAM(pos);
			pt.y = GET_Y_LPARAM(pos);
			ScreenToClient(s->hwnd, &pt);
			if (onSplitter(s, pt.x, pt.y)) {
				SetCursor(s->cursor);
				return 1;
			}
		}
		break;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, splitWndProc, uIdSubclass) == FALSE)
			logLastError(L"error removing uiSplit subclass");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

static void splitPadding(uiSplit *b, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	uiWindowsGetSizing(b->hwnd, &sizing);
	uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
}

static void splitRelayout(uiSplit *s)
{
	RECT r;
	int x, y, width, height;
	int xpadding, ypadding;
	int nStretchy;
	int stretchywid, stretchyht;
	int minWidth, minHeight;
	int nVisible;
	bool resetRatio = false;

	if (s->controls->size() == 0)
		return;

	uiWindowsEnsureGetClientRect(s->hwnd, &r);
	width = r.right - r.left;
	height = r.bottom - r.top;

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	nVisible = 0;
	for (struct splitChild &bc : *(s->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		nVisible++;
		if (bc.stretchy) {
			nStretchy++;
			uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minWidth, &minHeight);
			bc.height = minHeight;
			bc.width = minWidth;
			continue;
		}
		uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minWidth, &minHeight);
		if (s->vertical) {		// all controls have same width
			bc.width = width;
			bc.height = minHeight;
			stretchyht -= minHeight;
		} else {				// all controls have same height
			bc.width = minWidth;
			bc.height = height;
			stretchywid -= minWidth;
		}
	}
	if (nVisible == 0)
		return;

	// subtract spacer padding if needed
	splitPadding(s, &xpadding, &ypadding);
	if (s->vertical) {
		height -= (nVisible - 1) * ypadding;
		stretchyht -= (nVisible - 1) * ypadding;
	} else {
		width -= (nVisible - 1) * xpadding;
		stretchywid -= (nVisible - 1) * xpadding;
	}

	// 3) now get the size of stretchy controls and make sure to not shrink unshrinkable controls
	if (s->controls->size() == 1) {
		if (s->vertical)
			s->controls->at(0).height = stretchyht;
		else
			s->controls->at(0).width = stretchywid;
	} else {
		if (s->vertical) {
			int height0 = stretchyht * s->ratio;

			if (!s->controls->at(0).shrink && height0 < s->controls->at(0).height) {
				resetRatio = true;
				height0 = s->controls->at(0).height;
			}
			if (!s->controls->at(1).shrink && stretchyht - height0 < s->controls->at(1).height) {
				resetRatio = true;
				height0 = stretchyht - s->controls->at(1).height;
			}

			s->controls->at(0).height = height0;
			s->controls->at(1).height = stretchyht - height0;
		} else {
			int width0 = stretchywid * s->ratio;

			if (!s->controls->at(0).shrink && width0 < s->controls->at(0).width) {
				resetRatio = true;
				width0 = s->controls->at(0).width;
			}
			if (!s->controls->at(1).shrink && stretchywid - width0 < s->controls->at(1).width) {
				resetRatio = true;
				width0 = stretchywid - s->controls->at(1).width;
			}

			s->controls->at(0).width = width0;
			s->controls->at(1).width = stretchywid - width0;
		}
	}

	// reset ratio to account for no shrink adjustments
	if (resetRatio) {
		if (s->vertical)
			s->ratio = 1.0 * s->controls->at(0).height / stretchyht;
		else
			s->ratio = 1.0 * s->controls->at(0).width / stretchywid;
	}

	// position controls
	x = 0;
	y = 0;
	for (const struct splitChild &sc : *(s->controls)) {
		if (!uiControlVisible(sc.c))
			continue;
		uiWindowsEnsureMoveWindowDuringResize((HWND) uiControlHandle(sc.c), x, y, sc.width, sc.height);
		if (s->vertical)
			y += sc.height + ypadding;
		else
			x += sc.width + xpadding;
	}
}

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *b = uiSplit(c);

	for (const struct splitChild &bc : *(b->controls)) {
		uiControlSetParent(bc.c, NULL);
		uiControlDestroy(bc.c);
	}
	delete b->controls;
	uiWindowsEnsureDestroyWindow(b->hwnd);
	uiFreeControl(uiControl(b));
}

uiWindowsControlDefaultHandle(uiSplit)
uiWindowsControlDefaultParent(uiSplit)
uiWindowsControlDefaultSetParent(uiSplit)
uiWindowsControlDefaultToplevel(uiSplit)
uiWindowsControlDefaultVisible(uiSplit)
uiWindowsControlDefaultShow(uiSplit)
uiWindowsControlDefaultHide(uiSplit)
uiWindowsControlDefaultEnabled(uiSplit)
uiWindowsControlDefaultEnable(uiSplit)
uiWindowsControlDefaultDisable(uiSplit)

static void uiSplitSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiSplit *b = uiSplit(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(b), enabled))
		return;
	for (const struct splitChild &bc : *(b->controls))
		uiWindowsControlSyncEnableState(uiWindowsControl(bc.c), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiSplit)

static void uiSplitMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiSplit *b = uiSplit(c);
	int xpadding, ypadding;
	int nStretchy;
	// these two contain the largest minimum width and height of all stretchy controls in the split
	// all stretchy controls will use this value to determine the final minimum size
	int maxStretchyWidth, maxStretchyHeight;
	int minimumWidth, minimumHeight;
	int nVisible;

	*width = 0;
	*height = 0;
	if (b->controls->size() == 0)
		return;

	// 0) get this Split's padding
	splitPadding(b, &xpadding, &ypadding);

	// 1) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxStretchyWidth = 0;
	maxStretchyHeight = 0;
	nVisible = 0;
	for (const struct splitChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		nVisible++;
		uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minimumWidth, &minimumHeight);
		if (bc.stretchy) {
			nStretchy++;
			if (maxStretchyWidth < minimumWidth)
				maxStretchyWidth = minimumWidth;
			if (maxStretchyHeight < minimumHeight)
				maxStretchyHeight = minimumHeight;
		}
		if (b->vertical) {
			if (*width < minimumWidth)
				*width = minimumWidth;
			if (!bc.stretchy)
				*height += minimumHeight;
		} else {
			if (!bc.stretchy)
				*width += minimumWidth;
			if (*height < minimumHeight)
				*height = minimumHeight;
		}
	}
	if (nVisible == 0)		// just return 0x0
		return;

	// 2) now outset the desired rect with the needed padding
	if (b->vertical)
		*height += (nVisible - 1) * ypadding;
	else
		*width += (nVisible - 1) * xpadding;

	// 3) and now we can add in stretchy controls
	if (b->vertical)
		*height += nStretchy * maxStretchyHeight;
	else
		*width += nStretchy * maxStretchyWidth;
}

static void uiSplitMinimumSizeChanged(uiWindowsControl *c)
{
	uiSplit *b = uiSplit(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(b))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(b));
		return;
	}
	splitRelayout(b);
}

uiWindowsControlDefaultLayoutRect(uiSplit)
uiWindowsControlDefaultAssignControlIDZOrder(uiSplit)

static void uiSplitChildVisibilityChanged(uiWindowsControl *c)
{
	// TODO eliminate the redundancy
	uiWindowsControlMinimumSizeChanged(c);
}

static void splitArrangeChildren(uiSplit *b)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	for (const struct splitChild &bc : *(b->controls))
		uiWindowsControlAssignControlIDZOrder(uiWindowsControl(bc.c), &controlID, &insertAfter);
}

static void uiSplitAppend(uiSplit *b, uiControl *c, int stretchy, int shrink)
{
	struct splitChild bc;

	bc.c = c;
	bc.stretchy = stretchy;
	bc.shrink = shrink;
	uiControlSetParent(bc.c, uiControl(b));
	uiWindowsControlSetParentHWND(uiWindowsControl(bc.c), b->hwnd);
	b->controls->push_back(bc);
	splitArrangeChildren(b);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

void uiSplitDelete(uiSplit *b, int index)
{
	uiControl *c;

	c = (*(b->controls))[index].c;
	uiControlSetParent(c, NULL);
	uiWindowsControlSetParentHWND(uiWindowsControl(c), NULL);
	b->controls->erase(b->controls->begin() + index);
	splitArrangeChildren(b);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

static void onResize(uiWindowsControl *c)
{
	splitRelayout(uiSplit(c));
}

static uiSplit *uiNewSplit(int vertical)
{
	uiSplit *s;

	uiWindowsNewControl(uiSplit, s);
	s->hwnd = uiWindowsMakeContainer(uiWindowsControl(s), onResize);

	s->vertical = vertical;
	s->controls = new std::vector<struct splitChild>;
	s->moving = 0;
	s->ratio = 0.5;
	if (s->vertical)
		s->cursor = LoadCursorW(NULL, IDC_SIZENS);
	else
		s->cursor = LoadCursorW(NULL, IDC_SIZEWE);

	if (SetWindowSubclass(s->hwnd, splitWndProc, 0, (DWORD_PTR) s) == FALSE)
		logLastError(L"error subclassing uiSplit to handle parent messages");

	return s;
}

void uiSplitAdd1(uiSplit *s, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(s, child, expand, shrink);
}

void uiSplitAdd2(uiSplit *s, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(s, child, expand, shrink);
}

uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(0);
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(1);
}
