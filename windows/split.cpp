#include "uipriv_windows.hpp"

struct splitChild {
	uiControl *c;
	int stretchy;
	int width;
	int height;
};

struct uiSplit {
	uiWindowsControl c;
	HWND hwnd;
	HCURSOR hCursor;
	std::vector<struct splitChild> *controls;
	int vertical;
	int padded;
	int moving;
	int splitX;
	int splitY;
};

static void onResize(uiWindowsControl *c);

/********************* uiSplit WNDCLASS *********************/

static LRESULT CALLBACK splitWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	MINMAXINFO *mmi = (MINMAXINFO *) lParam;
	struct containerInit *init;
	uiWindowsControl *c;
	int minwid, minht;
	LRESULT lResult;
	uiSplit *s;

	s = (uiSplit *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	printf("uMsg %d\n", uMsg);
	switch (uMsg) {
	case WM_CREATE:
		if (s == NULL) {
			s = (uiSplit *) (cs->lpCreateParams);
			s->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) s);
		}
		break;
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
		s->moving = 1;
		SetCapture(hwnd);
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		s->moving = 0;
		return 0;

	case WM_MOUSEMOVE:
		if (s->moving && wParam == MK_LBUTTON)
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			/*
			if (HIWORD(lParam) > rect.bottom)
				return 0;

			dwSplitterPos = HIWORD(lParam);
			SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
			*/
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (x > rect.right || y > rect.bottom)
				return 0;
			s->splitX = x;
			s->splitY = y;
			printf("Mouse pos: %d %d\n", x, y);
		}
		return 0;
	}
	/*
	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_CREATE:
		init = (struct containerInit *) (cs->lpCreateParams);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (init->onResize));
		SetWindowLongPtrW(hwnd, 0, (LONG_PTR) (init->c));
		break;		// defer to DefWindowProc()
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;	// defer to DefWindowProc();
		onResize = (void (*)(uiWindowsControl *)) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		c = (uiWindowsControl *) GetWindowLongPtrW(hwnd, 0);
		(*(onResize))(c);
		return 0;
	case WM_GETMINMAXINFO:
		lResult = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		c = (uiWindowsControl *) GetWindowLongPtrW(hwnd, 0);
		uiWindowsControlMinimumSize(c, &minwid, &minht);
		mmi->ptMinTrackSize.x = minwid;
		mmi->ptMinTrackSize.y = minht;
		return lResult;
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
	}
	*/
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}



ATOM registerSplitHClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = splitHClass;
	wc.lpfnWndProc = splitWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.cbWndExtra = sizeof (void *);
	return RegisterClassW(&wc);
}

void unregisterSplitHClass(void)
{
	if (UnregisterClassW(splitHClass, hInstance) == 0)
		logLastError(L"error unregistering uiSplit window class");
}

/********************* uiSplit *********************/

static void splitPadding(uiSplit *b, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	*xpadding = 0;
	*ypadding = 0;
	if (b->padded) {
		uiWindowsGetSizing(b->hwnd, &sizing);
		uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
	}
}

static void splitRelayout(uiSplit *b)
{
	RECT r;
	int x, y, width, height;
	int xpadding, ypadding;
	int nStretchy;
	int stretchywid, stretchyht;
	int minimumWidth, minimumHeight;
	int nVisible;

	if (b->controls->size() == 0)
		return;

	uiWindowsEnsureGetClientRect(b->hwnd, &r);
	x = r.left;
	y = r.top;
	width = r.right - r.left;
	height = r.bottom - r.top;

	// -1) get this Split's padding
	splitPadding(b, &xpadding, &ypadding);

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	nVisible = 0;
	for (struct splitChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		nVisible++;
		if (bc.stretchy) {
			nStretchy++;
			continue;
		}
		uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minimumWidth, &minimumHeight);
		if (b->vertical) {		// all controls have same width
			bc.width = width;
			bc.height = minimumHeight;
			stretchyht -= minimumHeight;
		} else {				// all controls have same height
			bc.width = minimumWidth;
			bc.height = height;
			stretchywid -= minimumWidth;
		}
	}
	if (nVisible == 0)			// nothing to do
		return;

	// 2) now inset the available rect by the needed padding
	if (b->vertical) {
		height -= (nVisible - 1) * ypadding;
		stretchyht -= (nVisible - 1) * ypadding;
	} else {
		width -= (nVisible - 1) * xpadding;
		stretchywid -= (nVisible - 1) * xpadding;
	}

	// 3) now get the size of stretchy controls
	if (nStretchy != 0) {
		if (b->vertical)
			stretchyht /= nStretchy;
		else
			stretchywid /= nStretchy;
		for (struct splitChild &bc : *(b->controls)) {
			if (!uiControlVisible(bc.c))
				continue;
			if (bc.stretchy) {
				bc.width = stretchywid;
				bc.height = stretchyht;
			}
		}
	}

	// 4) now we can position controls
	// first, make relative to the top-left corner of the container
	x = 0;
	y = 0;
	for (const struct splitChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		uiWindowsEnsureMoveWindowDuringResize((HWND) uiControlHandle(bc.c), x, y, bc.width, bc.height);
		if (b->vertical)
			y += bc.height + ypadding;
		else
			x += bc.width + xpadding;
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

void uiSplitAppend(uiSplit *b, uiControl *c, int stretchy)
{
	struct splitChild bc;

	bc.c = c;
	bc.stretchy = stretchy;
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

int uiSplitNumChildren(uiSplit *b)
{
	return (int) b->controls->size();
}

int uiSplitPadded(uiSplit *b)
{
	return b->padded;
}

void uiSplitSetPadded(uiSplit *b, int padded)
{
	b->padded = padded;
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

	//s->hwnd = uiWindowsMakeContainer(uiWindowsControl(s), onResize);

	s->vertical = vertical;
	s->controls = new std::vector<struct splitChild>;

	if (s->vertical)
		s->hCursor = LoadCursorW(NULL, IDC_SIZENS);
	else
		s->hCursor = LoadCursorW(NULL, IDC_SIZEWE);

	// s->hwnd is assigned in splitWndProc()
	uiWindowsEnsureCreateControlHWND(0,
			splitHClass, L"",
			0,
			hInstance, s,
			FALSE);

	uiSplitSetPadded(s, 1);

	return s;
}

void uiSplitAdd1(uiSplit *s, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(s, child, expand);
}

void uiSplitAdd2(uiSplit *s, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(s, child, expand);
}

uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(0);
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(1);
}
