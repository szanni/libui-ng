#include "unit.h"

static void windowNew(void **state)
{
	uiWindow *w;
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	w = uiNewWindow("Unit Test", UNIT_TEST_WINDOW_WIDTH, UNIT_TEST_WINDOW_HEIGHT, 0);
	assert_non_null(w);
	uiWindowOnClosing(w, unitWindowOnClosingQuit, NULL);

	uiControlShow(uiControl(w));
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(w));

	uiUninit();
}

#define uiWindowFromState(s) (((struct state *)(*s))->w)

static void windowDefaultTitle(void **state)
{
	uiWindow *w = uiWindowFromState(state);
	char *rv;

	rv = uiWindowTitle(w);
	assert_string_equal(rv, "Unit Test");
	uiFreeText(rv);
}

static void windowSetTitle(void **state)
{
	uiWindow *w = uiWindowFromState(state);
	char *rv;

	uiWindowSetTitle(w, "SetTitle");
	rv = uiWindowTitle(w);
	assert_string_equal(rv, "SetTitle");
	uiFreeText(rv);
}

static void windowDefaultContentSize(void **state)
{
	uiWindow *w = uiWindowFromState(state);
	int x;
	int y;

	uiWindowContentSize(w, &x, &y);
	assert_int_equal(x, UNIT_TEST_WINDOW_WIDTH);
	assert_int_equal(y, UNIT_TEST_WINDOW_HEIGHT);
}


static void windowSetContentSize(void **state)
{
	uiWindow *w = uiWindowFromState(state);
	int x = UNIT_TEST_WINDOW_WIDTH + 1;
	int y = UNIT_TEST_WINDOW_HEIGHT + 1;

	uiWindowSetContentSize(w, x, y);
	uiWindowContentSize(w, &x, &y);
	assert_int_equal(x, UNIT_TEST_WINDOW_WIDTH + 1);
	assert_int_equal(y, UNIT_TEST_WINDOW_HEIGHT + 1);
}

static void windowDefaultFullscreen(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	assert_false(uiWindowFullscreen(w));
}

static void windowSetFullscreen(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetFullscreen(w, 1);
	assert_true(uiWindowFullscreen(w));
	uiWindowSetFullscreen(w, 0);
	assert_false(uiWindowFullscreen(w));
}

static void windowDefaultBorderless(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	assert_false(uiWindowBorderless(w));
}

static void windowSetBorderless(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetBorderless(w, 1);
	assert_true(uiWindowBorderless(w));
	uiWindowSetBorderless(w, 0);
	assert_false(uiWindowBorderless(w));
}

static void windowSetFullscreenBorderless(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetBorderless(w, 1);
	assert_true(uiWindowBorderless(w));
	assert_false(uiWindowFullscreen(w));

	uiWindowSetFullscreen(w, 1);
	assert_true(uiWindowBorderless(w));
	assert_true(uiWindowFullscreen(w));

	uiWindowSetFullscreen(w, 0);
	assert_true(uiWindowBorderless(w));
	assert_false(uiWindowFullscreen(w));
}

static void windowSetBorderlessFullscreen(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetFullscreen(w, 1);
	assert_false(uiWindowBorderless(w));
	assert_true(uiWindowFullscreen(w));

	uiWindowSetBorderless(w, 1);
	assert_true(uiWindowBorderless(w));
	assert_true(uiWindowFullscreen(w));

	uiWindowSetFullscreen(w, 0);
	assert_true(uiWindowBorderless(w));
	assert_false(uiWindowFullscreen(w));
}

static void windowDefaultMargined(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	assert_false(uiWindowMargined(w));
}

static void windowSetMargined(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetMargined(w, 1);
	assert_true(uiWindowMargined(w));
	uiWindowSetMargined(w, 0);
	assert_false(uiWindowMargined(w));
}

static void windowDefaultResizeable(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	assert_true(uiWindowResizeable(w));
}

static void windowSetResizeable(void **state)
{
	uiWindow *w = uiWindowFromState(state);

	uiWindowSetResizeable(w, 1);
	assert_true(uiWindowResizeable(w));
	uiWindowSetResizeable(w, 0);
	assert_false(uiWindowResizeable(w));
}

static int windowTestSetup(void **_state)
{
	struct state *state = *_state;
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	state->w = uiNewWindow("Unit Test", UNIT_TEST_WINDOW_WIDTH, UNIT_TEST_WINDOW_HEIGHT, 0);
	assert_non_null(state->w);
	uiWindowOnClosing(state->w, unitWindowOnClosingQuit, NULL);

	uiControlShow(uiControl(state->w));
	return 0;
}

static int windowTestTeardown(void **_state)
{
	struct state *state = *_state;

	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(state->w));
	uiUninit();
	return 0;
}

#define windowUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		windowTestSetup, windowTestTeardown)

int windowRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(windowNew),
		windowUnitTest(windowDefaultTitle),
		windowUnitTest(windowSetTitle),
		windowUnitTest(windowDefaultContentSize),
		windowUnitTest(windowSetContentSize),
		windowUnitTest(windowDefaultFullscreen),
		windowUnitTest(windowSetFullscreen),
		windowUnitTest(windowDefaultBorderless),
		windowUnitTest(windowSetBorderless),
		windowUnitTest(windowSetFullscreenBorderless),
		windowUnitTest(windowSetBorderlessFullscreen),
		windowUnitTest(windowDefaultMargined),
		windowUnitTest(windowSetMargined),
		windowUnitTest(windowDefaultResizeable),
		windowUnitTest(windowSetResizeable),
	};

	return cmocka_run_group_tests_name("uiWindow", tests, unitTestsSetup, unitTestsTeardown);
}

