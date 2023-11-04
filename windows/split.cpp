#include "uipriv_windows.hpp"

struct splitChild {
	uiControl *c;
	int width;
	int height;
	bool shrink;
};

struct uiSplit {
	uiWindowsControl c;
	HWND hwnd;
	float ratio;
	int mouseOffset;
	HCURSOR cursor;
	struct splitChild controls[2];
	bool vertical;
	bool moving;
};

static void splitPadding(uiSplit *b, int *xpadding, int *ypadding);
static void splitRelayout(uiSplit *s);

static int nControlsVisible(uiSplit *s)
{
	int n = 0;

	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL || !uiControlVisible(sc->c))
			continue;
		n++;
	}
	return n;
}

static bool cursorOnSplitter(uiSplit *s, int px, int py)
{
	int xpadding, ypadding;
	splitPadding(s, &xpadding, &ypadding);

	if (nControlsVisible(s) != 2)
		return false;

	int x = s->controls[0].width;
	int y = s->controls[0].height;
	if (s->vertical) {
		if (py >= y && py < y + ypadding)
			return true;
	} else {
		if (px >= x && px < x + xpadding)
			return true;
	}
	return false;
}

static LRESULT CALLBACK splitWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiSplit *s = uiSplit(dwRefData);

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			if (nControlsVisible(s) != 2)
				return 0;

			if (s->vertical)
				s->mouseOffset = y - s->controls[0].height;
			else
				s->mouseOffset = x - s->controls[0].width;

			s->moving = true;
			SetCapture(hwnd);
		}
		return 0;
	case WM_LBUTTONUP:
		if (s->moving) {
			s->moving = false;
			ReleaseCapture();
		}
		return 0;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON && s->moving) {
			RECT r;
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			GetClientRect(hwnd, &r);
			if (s->vertical && y - s->mouseOffset > r.bottom)
				return 0;
			if (!s->vertical && x - s->mouseOffset > r.right)
				return 0;

			if (s->vertical)
				s->ratio = 1.0 * (y - s->mouseOffset) / r.bottom;
			else
				s->ratio = 1.0 * (x - s->mouseOffset) / r.right;

			if (s->ratio < 0.0)
				s->ratio = 0.0;
			if (s->ratio > 1.0)
				s->ratio = 1.0;

			splitRelayout(s);
		}
		return 0;
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
			if (cursorOnSplitter(s, pt.x, pt.y)) {
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

static void splitPadding(uiSplit *s, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	uiWindowsGetSizing(s->hwnd, &sizing);
	uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
}

static void splitRelayout(uiSplit *s)
{
	RECT r;
	int x, y, width, height;
	int xpadding, ypadding;
	int minWidth, minHeight;
	int nVisible = 0;
	bool resetRatio = false;

	uiWindowsEnsureGetClientRect(s->hwnd, &r);
	width = r.right - r.left;
	height = r.bottom - r.top;

	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL || !uiControlVisible(sc->c))
			continue;

		uiWindowsControlMinimumSize(uiWindowsControl(sc->c), &minWidth, &minHeight);
		if (s->vertical) {
			sc->width = width;
			sc->height = minHeight;
		} else {
			sc->width = minWidth;
			sc->height = height;
		}
		nVisible++;
	}

	if (nVisible == 0)
		return;

	splitPadding(s, &xpadding, &ypadding);

	if (nVisible == 1) {
		for (int i = 0; i < 2; i++) {
			struct splitChild *sc = &s->controls[i];
			if (sc->c == NULL || !uiControlVisible(sc->c))
				continue;

			if (s->vertical)
				sc->height = height;
			else
				sc->width = width;
		}
	} else {
		if (s->vertical) {
			height -= ypadding;
			int height0 = height * s->ratio;

			if (!s->controls[0].shrink && height0 < s->controls[0].height) {
				resetRatio = true;
				height0 = s->controls[0].height;
			}
			if (!s->controls[1].shrink && height - height0 < s->controls[1].height) {
				resetRatio = true;
				height0 = height - s->controls[1].height;
			}

			s->controls[0].height = height0;
			s->controls[1].height = height - height0;
		} else {
			width -= xpadding;
			int width0 = width * s->ratio;
			if (!s->controls[0].shrink && width0 < s->controls[0].width) {
				resetRatio = true;
				width0 = s->controls[0].width;
			}
			if (!s->controls[1].shrink && width - width0 < s->controls[1].width) {
				resetRatio = true;
				width0 = width - s->controls[1].width;
			}

			s->controls[0].width = width0;
			s->controls[1].width = width - width0;
		}
	}

	// reset ratio to account for no shrink adjustments
	if (resetRatio) {
		if (s->vertical)
			s->ratio = 1.0 * s->controls[0].height / height;
		else
			s->ratio = 1.0 * s->controls[0].width / width;
	}

	// position controls
	x = 0;
	y = 0;
	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL || !uiControlVisible(sc->c))
			continue;

		uiWindowsEnsureMoveWindowDuringResize((HWND) uiControlHandle(sc->c), x, y, sc->width, sc->height);
		if (s->vertical)
			y += sc->height + ypadding;
		else
			x += sc->width + xpadding;
	}
}

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *s = uiSplit(c);

	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL)
			continue;
		uiControlSetParent(sc->c, NULL);
		uiControlDestroy(sc->c);
	}
	uiWindowsEnsureDestroyWindow(s->hwnd);
	uiFreeControl(uiControl(s));
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
	uiSplit *s = uiSplit(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(s), enabled))
		return;

	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL)
			continue;
		uiWindowsControlSyncEnableState(uiWindowsControl(sc->c), enabled);
	}
}

uiWindowsControlDefaultSetParentHWND(uiSplit)

static void uiSplitMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiSplit *s = uiSplit(c);
	int xpadding, ypadding;
	int minWidth, minHeight;
	int nVisible = 0;

	*width = 0;
	*height = 0;

	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL || !uiControlVisible(sc->c))
			continue;

		uiWindowsControlMinimumSize(uiWindowsControl(sc->c), &minWidth, &minHeight);
		if (s->vertical) {
			if (*width < minWidth)
				*width = minWidth;
			*height += minHeight;
		} else {
			if (*height < minHeight)
				*height = minHeight;
			*width += minWidth;
		}
		nVisible++;
	}

	if (nVisible == 2) {
		splitPadding(s, &xpadding, &ypadding);

		if (s->vertical)
			*height += ypadding;
		else
			*width += xpadding;
	}
}

static void uiSplitMinimumSizeChanged(uiWindowsControl *c)
{
	uiSplit *s = uiSplit(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(s))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(s));
		return;
	}
	splitRelayout(s);
}

uiWindowsControlDefaultLayoutRect(uiSplit)
uiWindowsControlDefaultAssignControlIDZOrder(uiSplit)

static void uiSplitChildVisibilityChanged(uiWindowsControl *c)
{
	// TODO eliminate the redundancy
	uiWindowsControlMinimumSizeChanged(c);
}

static void splitArrangeChildren(uiSplit *s)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	for (int i = 0; i < 2; i++) {
		struct splitChild *sc = &s->controls[i];
		if (sc->c == NULL)
			continue;
		uiWindowsControlAssignControlIDZOrder(uiWindowsControl(sc->c), &controlID, &insertAfter);
	}
}

static void uiSplitInsertAt(uiSplit *s, uiControl *c, int i)
{
	struct splitChild *sc = &s->controls[i];

	sc->c = c;
	// for future uiSplitSet(First|Second)Shrink
	sc->shrink = false;
	uiControlSetParent(sc->c, uiControl(s));
	uiWindowsControlSetParentHWND(uiWindowsControl(sc->c), s->hwnd);
	splitArrangeChildren(s);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(s));
}

static void onResize(uiWindowsControl *c)
{
	splitRelayout(uiSplit(c));
}

static uiSplit *uiNewSplit(bool vertical)
{
	uiSplit *s;

	uiWindowsNewControl(uiSplit, s);
	s->hwnd = uiWindowsMakeContainer(uiWindowsControl(s), onResize);

	s->vertical = vertical;
	s->moving = false;
	s->ratio = 0.5;
	for (int i = 0; i < 2; i++) {
		s->controls[i].c = NULL;
	}

	if (s->vertical)
		s->cursor = LoadCursorW(NULL, IDC_SIZENS);
	else
		s->cursor = LoadCursorW(NULL, IDC_SIZEWE);

	if (SetWindowSubclass(s->hwnd, splitWndProc, 0, (DWORD_PTR) s) == FALSE)
		logLastError(L"error subclassing uiSplit to handle parent messages");

	return s;
}

void uiSplitSetFirst(uiSplit *s, uiControl *child)
{
	uiSplitInsertAt(s, child, 0);
}

void uiSplitSetSecond(uiSplit *s, uiControl *child)
{
	uiSplitInsertAt(s, child, 1);
}

uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(false);
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(true);
}
