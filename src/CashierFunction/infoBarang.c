#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"
#include "cashierFunction.h"

void infoBarang(http_event* e) {
    char barangID[11];
    sqlite3* db;
    SQLRow allRow = {0};
    char strTemp[255];
    char* errMsg;

    http_get_header(e, "barangID", barangID, 10);

    if (!barangID[0]) {
        http_send_status(e, 403, "Mohon input ID Barang yang benar!");
        return;
    }
    for (int a = 0; a < strlen(barangID); a++) if (!isdigit(barangID[a])) {
        http_send_status(e, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    sprintf(strTemp, "SELECT rowid,* from daftarBarang where rowid=%d", atoi(barangID));

    if (sqlite3_exec(db, strTemp, RowBack, &allRow, &errMsg) != SQLITE_OK) {
        freeRowBack(&allRow);
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at infoBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    } else if (!allRow.rows) {
        freeRowBack(&allRow);
        http_send_status(e, 403, "Barang tersebut tidak tersedia di Daftar Barang, mohon input ID Barang yang benar!");
        sqlite3_close(db);
        return;
    }

    http_write(e, allRow.rows, allRow.totalChar);
    freeRowBack(&allRow);
    sqlite3_close(db);

    return;
}
