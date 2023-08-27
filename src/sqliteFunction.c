#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vendor/sqlite3/sqlite3.h"
#include "sqliteFunction.h"
#include "../vendor/httplib/httplibrary.h"

int tableExists(void *data, int argc, char **argv, char **azColName) {
    if (argv[0]) *(char*)data = 1;
    return 0;
}

void freeRowBack(SQLRow* data) {
    if (data->totalChar) free(data->rows);
    memset(data, 0, sizeof(SQLRow));
}

int RowBack(void* data, int argc, char** argv, char** colName) {
    SQLRow* rowData = data;

    if (!argv[0]) {
        return 0;
    }
    for (int a = 0; a < argc; a++) {
        if (!rowData->rows) {
            rowData->rows = malloc(strlen(argv[a]) + 2);
            strcpy(rowData->rows, argv[a]);
            rowData->rows[strlen(argv[a])] = '|';
            rowData->totalChar += strlen(argv[a]) + 1;
        } else {
            rowData->rows = realloc(rowData->rows, (strlen(argv[a]) + rowData->totalChar + 2) * 2);
            strcpy(rowData->rows + rowData->totalChar, argv[a]);
            rowData->rows[strlen(argv[a]) + rowData->totalChar] = '|';
            rowData->totalChar += strlen(argv[a]) + 1;
        }
    }

    rowData->rows[rowData->totalChar - 1] = '\n';
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

int sqlNormalExec(http_event* e, sqlite3* db, char* tempString) {
    char* errMsg;

    if (sqlite3_exec(db, tempString, 0, 0, &errMsg) != SQLITE_OK) {
        printf("%s\n", errMsg);
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }

    return 1;
}

int sqlBackExec(http_event* e, sqlite3* db, char* tempString, int (*callback)(void *, int, char **, char **), void* var) {
    char* errMsg;

    if (sqlite3_exec(db, tempString, callback, var, &errMsg)) {
        printf("%s\n", errMsg);
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }

    return 1;
}
