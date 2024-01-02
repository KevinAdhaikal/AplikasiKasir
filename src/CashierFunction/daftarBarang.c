#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"

#include "cashierFunction.h"

int daftarBarang(sb_Event* e) {
    char* errMsg;
    sqlite3* db;
    SQLRow allRows = {0};
    sqlite3_stmt* statement;

    sqlite3_open("database/daftarBarang.db", &db);
    sqlite3_prepare_v2(db, "SELECT rowid,* from daftarBarang", -1, &statement, NULL);
    statement_get_row(statement, &allRows, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    sb_write(e->stream, allRows.rows, allRows.totalChar);
    freeRowBack(&allRows);
    return SB_RES_OK;
}
