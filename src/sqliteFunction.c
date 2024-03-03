#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../vendor/sqlite3/sqlite3.h"
#include "sqliteFunction.h"
#include "../vendor/sandbird/sandbird.h"

int tableExists(void *data, int argc, char **argv, char **azColName) {
    if (argv[0]) *(char*)data = 1;
    return 0;
}

char check_is_row(sqlite3* db, const char* sql) {
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, sql, -1, &statement, NULL);
    char res = sqlite3_step(statement) == SQLITE_ROW;
    printf("%d\n", res);
    sqlite3_finalize(statement);
    return res;
}

void freeRowBack(SQLRow* data) {
    if (data->totalChar > 0) free(data->rows);
    data->rows = NULL;
    memset(data, 0, sizeof(SQLRow));
}

int RowBack(void* data, int argc, char** argv, char** colName) {
    SQLRow* rowData = data;

    if (!argv[0]) {
        return 0;
    }
    for (int a = 0; a < argc; a++) {
        if (!rowData->rows) {
            if (!argv[a]) {
                rowData->rows = malloc(6);
                memcpy(rowData->rows, "NULL\x02", 5);
                rowData->totalChar += 5;
                continue;
            }
            rowData->rows = malloc(strlen(argv[a]) + 2);
            strcpy(rowData->rows, argv[a]);
            rowData->rows[strlen(argv[a])] = '|';
            rowData->totalChar += strlen(argv[a]) + 1;
        } else {
            if (!argv[a]) {
                rowData->rows = realloc(rowData->rows, (rowData->totalChar + 6) * 2);
                strcpy(rowData->rows + rowData->totalChar, "NULL");
                rowData->rows[4 + rowData->totalChar] = '|';
                rowData->totalChar += 5;
                continue;
            }
            rowData->rows = realloc(rowData->rows, (strlen(argv[a]) + rowData->totalChar + 2) * 2);
            strcpy(rowData->rows + rowData->totalChar, argv[a]);
            rowData->rows[strlen(argv[a]) + rowData->totalChar] = '|';
            rowData->totalChar += strlen(argv[a]) + 1;
        }
    }

    rowData->rows[rowData->totalChar - 1] = '\x01';
    rowData->rows[rowData->totalChar] = '\0';
    return 0;
}


int RowBackPlus(void* data, int argc, char** argv, char** colName) {
    for (int a = 0; a < argc; a++) {
        *(int*)data += atoi(argv[a]);
    }
    return 0;
}

int sqlReturnInt(void* data, int argc, char** argv, char** colName) {
    *(int*)data = atoi(argv[0]);
    return 0;
}

int sqlNormalExec(sb_Event* e, sqlite3* db, char* tempString) {
    char* errMsg;

    if (sqlite3_exec(db, tempString, 0, 0, &errMsg) != SQLITE_OK) {
        printf("%s\n", errMsg);
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    return 1;
}

int sqlBackExec(sb_Event* e, sqlite3* db, char* tempString, int (*callback)(void *, int, char **, char **), void* var) {
    char* errMsg;

    if (sqlite3_exec(db, tempString, callback, var, &errMsg)) {
        printf("%s\n", errMsg);
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    return 1;
}

int sqlTOF(void* data, int argc, char** argv, char** colName) {
    *(char*)data = 1;
    return 1;
}

char statement_get_row(sqlite3_stmt* statement, SQLRow* row, char no_lf) {
    row->sql_ret = sqlite3_step(statement);
    if (row->sql_ret == SQLITE_ROW) goto HAS_ROW;
    else return 0;
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        HAS_ROW:
        int columnCount = sqlite3_column_count(statement);
        if (!columnCount) return 0;
        for (int a = 0; a < columnCount; a++) {
            switch (sqlite3_column_type(statement, a)) {
                case SQLITE_TEXT: {
                    const char* text = (const char*)sqlite3_column_text(statement, a);
                    int str_len = strlen(text);

                    row->rows = realloc(row->rows, row->totalChar + str_len + 2);
                    snprintf(row->rows + row->totalChar, str_len + 2, "%s\x02", text);
                    row->totalChar += str_len + 1;
                    break;
                }
                case SQLITE_INTEGER: {
                    int64_t value = sqlite3_column_int64(statement, a);
                    char int_to_str[64];
                    char str_len = snprintf(int_to_str, sizeof(int_to_str), "%lld\x02", value);
                    row->rows = realloc(row->rows, row->totalChar + str_len + 1);
                    snprintf(row->rows + row->totalChar, str_len + 1, "%s", int_to_str);
                    
                    row->totalChar += str_len;
                    break;
                }
                case SQLITE_NULL: {
                    row->rows = realloc(row->rows, row->totalChar + 6);
                    memcpy(row->rows + row->totalChar, "NULL\x02", 5);
                    row->totalChar += 5;
                    break;
                }
            }
        }
        row->rows[row->totalChar - 1] = 0x01;
    }
    if (row->totalChar) row->rows[no_lf == 1 ? --row->totalChar : row->totalChar] = '\0';

    return 1;
} // thanks ChatGPT

int64_t statement_row_plus(sqlite3_stmt* statement) {
    int64_t result = 0;

    while(sqlite3_step(statement) == SQLITE_ROW) {
        int columnCount = sqlite3_column_count(statement);
        if (!columnCount) return 0;

        for (int a = 0; a < columnCount; a++) {
            switch(sqlite3_column_type(statement, a)) {
                case SQLITE_INTEGER: {
                    result += sqlite3_column_int64(statement, a);
                    break;
                }
            }
        }
    }

    return result;
}