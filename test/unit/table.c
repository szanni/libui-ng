#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "unit.h"

#define uiTablePtrFromState(s) uiControlPtrFromState(uiTable, s)

struct row {
	char string[32];
};

struct data {
	int numRows;
	struct row *rows;
};

enum MCOL {
	MCOL_STRING,
	MCOL_SIZE
};

static struct data data;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return MCOL_SIZE;
}

static uiTableValueType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	switch (column) {
	case MCOL_STRING:
		return uiTableValueTypeString;
	default:
		assert(0);
	}
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return data.numRows;
}

static uiTableValue *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int column)
{
	switch (column) {
	case MCOL_STRING:
		return uiNewTableValueString("Row");
	default:
		assert(0);
	}
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int column, const uiTableValue *value)
{
	//ignore
}

static uiTableModelHandler mh = {
	.NumColumns = modelNumColumns,
	.ColumnType = modelColumnType,
	.NumRows = modelNumRows,
	.CellValue = modelCellValue,
	.SetCellValue = modelSetCellValue,
};

static void modelNewData(void)
{
	int i;

	data.numRows = 10;
	data.rows = malloc(data.numRows * sizeof(*data.rows));
	assert_non_null(data.rows);

	for (i = 0; i < data.numRows; ++i) {
		sprintf(data.rows[i].string, "Row %d", i);
	}
}

static void modelFreeData(void)
{
	free(data.rows);
}

static void tableModelNew(void **state)
{
	uiTableModel *m;

	modelNewData();
	m = uiNewTableModel(&mh);
	uiFreeTableModel(m);
	modelFreeData();
}

static int onClosing(uiWindow *w, void *data)
{
        uiQuit();
        return 1;
}

int tableTestSetup(void **_state)
{
	struct state *state = *_state;
	uiInitOptions o = {0};

	assert_null(uiInit(&o));
	state->w = uiNewWindow("Unit Test", UNIT_TEST_WIDTH, UNIT_TEST_HEIGHT, 0);
	uiWindowOnClosing(state->w, onClosing, NULL);

	uiTableModel **m = uiDataPtrFromState(uiTableModel, _state);
	modelNewData();
	*m = uiNewTableModel(&mh);

	return 0;
}

int tableTestTeardown(void **_state)
{
	struct state *state = *_state;
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, _state);

	uiWindowSetChild(state->w, uiControl(state->c));
	uiControlShow(uiControl(state->w));
	uiMainSteps();
	uiMainStep(1);
	uiControlDestroy(uiControl(state->w));

	uiFreeTableModel(m);
	modelFreeData();

	uiUninit();

	return 0;
}

static void tableNew(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
}

static void tableAppendTextColumn(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);
}

static void tableAllowMultipleSelectionDefault(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	assert_int_equal(uiTableAllowMultipleSelection(*t), 0);
}

static void tableSetAllowMultipleSelection(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	uiTableSetAllowMultipleSelection(*t, 1);
	assert_int_equal(uiTableAllowMultipleSelection(*t), 1);
	uiTableSetAllowMultipleSelection(*t, 0);
	assert_int_equal(uiTableAllowMultipleSelection(*t), 0);
}

static void tableCurrentSelectionDefault(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection *sel;

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	sel = uiTableCurrentSelection(*t);
	assert_int_equal(sel->NumRows, 0);
	uiFreeTableSelection(sel);
}

static void tableSetCurrentSelectionToSingleAllowMultipleFalse(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection selSet;
	uiTableSelection *selGet;
	int row;

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	selSet.NumRows = 1;
	row = 0;
	selSet.Rows = &row;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 0);
	uiFreeTableSelection(selGet);

	selSet.NumRows = 1;
	row = 1;
	selSet.Rows = &row;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 1);
	uiFreeTableSelection(selGet);
}

static void tableSetCurrentSelectionToSingleAllowMultipleTrue(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection selSet;
	uiTableSelection *selGet;
	int row;

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	uiTableSetAllowMultipleSelection(*t, 1);

	selSet.NumRows = 1;
	row = 0;
	selSet.Rows = &row;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 0);
	uiFreeTableSelection(selGet);

	selSet.NumRows = 1;
	row = 1;
	selSet.Rows = &row;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 1);
	uiFreeTableSelection(selGet);
}

static void tableSetCurrentSelectionToMultipleAllowMultipleTrue(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection selSet;
	uiTableSelection *selGet;
	int rows[3];

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	uiTableSetAllowMultipleSelection(*t, 1);

	selSet.NumRows = 2;
	rows[0] = 0;
	rows[1] = 3;
	selSet.Rows = rows;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 2);
	assert_int_equal(selGet->Rows[0], 0);
	assert_int_equal(selGet->Rows[1], 3);
	uiFreeTableSelection(selGet);

	selSet.NumRows = 3;
	rows[0] = 5;
	rows[1] = 1;
	rows[2] = 3;
	selSet.Rows = rows;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 3);
	assert_int_equal(selGet->Rows[0], 1);
	assert_int_equal(selGet->Rows[1], 3);
	assert_int_equal(selGet->Rows[2], 5);
	uiFreeTableSelection(selGet);
}


/*
static void tableSetCurrentSelectionToMultipleAllowMultipleFalseSelectLast(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection selSet;
	uiTableSelection *selGet;
	int rows[2];

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	selSet.NumRows = 2;
	rows[0] = 0;
	rows[1] = 1;
	selSet.Rows = rows;
	uiTableSetCurrentSelection(*t, &selSet);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 1);
	uiFreeTableSelection(selGet);
}
*/

static void tableSetAllowMultipleSelectionFalseWhileMultipleSelected(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	uiTable **t = uiTablePtrFromState(state);
	uiTableSelection selSet;
	uiTableSelection *selGet;
	int rows[2];

	uiTableParams p = {.Model = m, .RowBackgroundColorModelColumn = -1};
	*t = uiNewTable(&p);
	uiTableAppendTextColumn(*t, "Column", MCOL_STRING, uiTableModelColumnNeverEditable, NULL);

	uiTableSetAllowMultipleSelection(*t, 1);

	selSet.NumRows = 2;
	rows[0] = 0;
	rows[1] = 1;
	selSet.Rows = rows;
	uiTableSetCurrentSelection(*t, &selSet);

	uiTableSetAllowMultipleSelection(*t, 0);

	selGet = uiTableCurrentSelection(*t);
	assert_int_equal(selGet->NumRows, 1);
	assert_int_equal(selGet->Rows[0], 1);
	uiFreeTableSelection(selGet);
}



#define tableUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		tableTestSetup, tableTestTeardown)

int tableRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(tableModelNew),
		tableUnitTest(tableNew),
		tableUnitTest(tableAppendTextColumn),
		tableUnitTest(tableAllowMultipleSelectionDefault),
		tableUnitTest(tableSetAllowMultipleSelection),
		tableUnitTest(tableCurrentSelectionDefault),
		tableUnitTest(tableSetCurrentSelectionToSingleAllowMultipleFalse),
		tableUnitTest(tableSetCurrentSelectionToSingleAllowMultipleTrue),
		tableUnitTest(tableSetCurrentSelectionToMultipleAllowMultipleTrue),
		//tableUnitTest(tableSetCurrentSelectionToMultipleAllowMultipleFalseSelectLast),
		tableUnitTest(tableSetAllowMultipleSelectionFalseWhileMultipleSelected),
	};

	return cmocka_run_group_tests_name("uiTable+Model", tests, unitTestsSetup, unitTestsTeardown);
}

