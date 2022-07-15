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

int tableTestSetup(void **state)
{
	int rv = unitTestSetup(state);
	uiTableModel **m = uiDataPtrFromState(uiTableModel, state);

	modelNewData();
	*m = uiNewTableModel(&mh);
	return rv;
}

int tableTestTeardown(void **state)
{
	uiTableModel *m = *uiDataPtrFromState(uiTableModel, state);
	int rv = unitTestTeardown(state);

	uiFreeTableModel(m);
	modelFreeData();
	return rv;
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

#define tableUnitTest(f) cmocka_unit_test_setup_teardown((f), \
		tableTestSetup, tableTestTeardown)

int tableRunUnitTests(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(tableModelNew),
		tableUnitTest(tableNew),
		tableUnitTest(tableAppendTextColumn),
	};

	return cmocka_run_group_tests_name("uiTable+Model", tests, unitTestsSetup, unitTestsTeardown);
}

