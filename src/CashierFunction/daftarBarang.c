#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/httplib/httplibrary.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"

#include "cashierFunction.h"

void daftarBarang(http_event* e) {
    FILE* fp = fopen("database/daftarBarang.db", "rb");
    if (fp) {
        fclose(fp);
        char* errMsg;
        sqlite3* db;
        SQLRow allRows = {0};

        sqlite3_open("database/daftarBarang.db", &db);

        if (sqlite3_exec(db, "SELECT rowid,* from daftarBarang", RowBack, &allRows, &errMsg) != SQLITE_OK) {
            freeRowBack(&allRows);

            if (!isStr(errMsg, "no such table", 0)) {
                http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                printf("[ERROR] Something wrong in SQLite at daftarBarang.c: %s\n", errMsg);
            }
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return;
        }

        http_write(e, allRows.rows, allRows.totalChar);
        freeRowBack(&allRows);
        sqlite3_close(db);
    }
    return;
}
