// 8 september 2015
#include "uipriv_windows.h"
#include "area.h"

static HRESULT doPaint(uiArea *a, ID2D1RenderTarget *rt, RECT *clip)
{
	uiAreaHandler *ah = a->ah;
	uiAreaDrawParams dp;
	COLORREF bgcolorref;
	D2D1_COLOR_F bgcolor;

	dp.Context = newContext(rt);

	loadAreaSize(a, rt, &(dp.AreaWidth), &(dp.AreaHeight));

	dp.ClipX = clip->left;
	dp.ClipY = clip->top;
	dp.ClipWidth = clip->right - clip->left;
	dp.ClipHeight = clip->bottom - clip->top;

	ID2D1RenderTarget_BeginDraw(rt);

	// TODO only clear the clip area
	// TODO clear with actual background brush
	bgcolorref = GetSysColor(COLOR_BTNFACE);
	bgcolor.r = ((float) GetRValue(bgcolorref)) / 255.0;
	// due to utter apathy on Microsoft's part, GetGValue() does not work with MSVC's Run-Time Error Checks
	// it has not worked since 2008 and they have *never* fixed it
	bgcolor.g = ((float) ((BYTE) ((bgcolorref & 0xFF00) >> 8))) / 255.0;
	bgcolor.b = ((float) GetBValue(bgcolorref)) / 255.0;
	bgcolor.a = 1.0;
	ID2D1RenderTarget_Clear(rt, &bgcolor);

	// no need to save or restore the graphics state to reset transformations; it's handled by resetTarget() in draw.c, called by newContext() above
	(*(ah->Draw))(ah, a, &dp);

	freeContext(dp.Context);

	return ID2D1RenderTarget_EndDraw(rt, NULL, NULL);
}

static void onWM_PAINT(uiArea *a)
{
	RECT clip;
	HRESULT hr;

	// do not clear the update rect; we do that ourselves in doPaint()
	if (GetUpdateRect(a->hwnd, &clip, FALSE) == 0) {
		// set a zero clip rect just in case GetUpdateRect() didn't change clip
		clip.left = 0;
		clip.top = 0;
		clip.right = 0;
		clip.bottom = 0;
	}
	hr = doPaint(a, (ID2D1RenderTarget *) (a->rt), &clip);
	switch (hr) {
	case S_OK:
		if (ValidateRect(a->hwnd, NULL) == 0)
			logLastError("error validating rect in onWM_PAINT()");
		break;
	case D2DERR_RECREATE_TARGET:
		// DON'T validate the rect
		// instead, simply drop the render target
		// we'll get another WM_PAINT and make the render target again
		// TODO would this require us to invalidate the entire client area?
		ID2D1HwndRenderTarget_Release(a->rt);
		a->rt = NULL;
		break;
	default:
		logHRESULT("error painting in onWM_PAINT()", hr);
	}
}

static void onWM_PRINTCLIENT(uiArea *a)
{
	RECT client;

	if (GetClientRect(a->hwnd, &client) == 0)
		logLastError("error getting client rect in onWM_PRINTCLIENT()");
//TODO	doPaint(a, (HDC) wParam, &client);
}

BOOL areaDoDraw(uiArea *a, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	switch (uMsg) {
	case WM_PAINT:
		onWM_PAINT(a);
		*lResult = 0;
		return TRUE;
	case WM_PRINTCLIENT:
		onWM_PRINTCLIENT(a);
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

// TODO only if the render target wasn't just created?
void areaDrawOnResize(uiArea *a, RECT *newClient)
{
	D2D1_SIZE_U size;

	size.width = newClient->right - newClient->left;
	size.height = newClient->bottom - newClient->top;
	// don't track the error; we'll get that in EndDraw()
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd370994%28v=vs.85%29.aspx
	ID2D1HwndRenderTarget_Resize(a->rt, &size);

	// according to Rick Brewster, we must always redraw the entire client area after calling ID2D1RenderTarget::Resize() (see http://stackoverflow.com/a/33222983/3408572)
	// we used to have a uiAreaHandler.RedrawOnResize() method to decide this; now you know why we don't anymore
	if (InvalidateRect(a->hwnd, NULL, TRUE) == 0)
		logLastError("error redrawing area on resize in areaDrawOnResize()");
}