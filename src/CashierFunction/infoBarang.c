#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "cashierFunction.h"

int infoBarang(sb_Event* e) {
    char barangID[11];
    sqlite3* db;
    SQLRow allRow = {0};
    char strTemp[255];
    char* errMsg;

    sb_get_header(e->stream, "barangID", barangID, 10);

    if (!barangID[0]) {
        sb_send_status(e->stream, 403, "Mohon input ID Barang yang benar!");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(barangID); a++) if (!isdigit(barangID[a])) {
        sb_send_status(e->stream, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    sprintf(strTemp, "SELECT rowid,* from daftarBarang where rowid=%d", atoi(barangID));

    if (sqlite3_exec(db, strTemp, RowBack, &allRow, &errMsg) != SQLITE_OK) {
        freeRowBack(&allRow);
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at infoBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    } else if (!allRow.rows) {
        freeRowBack(&allRow);
        sb_send_status(e->stream, 403, "Barang tersebut tidak tersedia di Daftar Barang, mohon input ID Barang yang benar!");
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sb_write(e->stream, allRow.rows, allRow.totalChar);
    freeRowBack(&allRow);
    sb_send_status(e->stream, 200, "OK");
    sqlite3_close(db);

    return SB_RES_OK;
}
