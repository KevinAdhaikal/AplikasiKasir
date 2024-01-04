#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

int hapusDaftarBarang(sb_Event* e) {
    char IDNumber[11], namaBarang[255], strTemp[512];
    SQLRow row = {0};
    sqlite3_stmt* statement;
    sqlite3* db;
    char* errMsg;

    sb_get_header(e->stream, "IDNumber", IDNumber, 10);

    if (!IDNumber[0]) {
        sb_send_status(e->stream, 403, "Mohon input ID Barang yang benar!");
        return SB_RES_OK;
    }
    uint8_t str_len = strlen(IDNumber);
    for (uint8_t a = 0; a < str_len; a++) if (!isdigit(IDNumber[a])) {
        sb_send_status(e->stream, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);

    sqlite3_prepare_v2(db, "SELECT nama FROM daftarBarang WHERE rowid = ?", -1 , &statement, NULL);
    sqlite3_bind_int(statement, 1, atoi(IDNumber));
    statement_get_row(statement, &row, 1);
    sqlite3_finalize(statement);
    if (!row.totalChar) {
        sb_send_status(e->stream, 403, "Ada yang salah dengan client! mohon lakukan refresh dengan tekan CTRL + R pada browser");
        return SB_RES_OK;
    }

    strcpy(namaBarang, row.rows);
    freeRowBack(&row);

    sqlite3_prepare_v2(db, "DELETE FROM daftarBarang WHERE rowid = ?", -1, &statement, NULL);
    sqlite3_bind_int(statement, 1, atoi(IDNumber));
    sqlite3_step(statement);
    sqlite3_finalize(statement);

    sqlite3_close(db);
    sqlite3_open("database/barangMasuk.db", &db);

    sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type = ?", -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, "table", -1, SQLITE_STATIC);
    statement_get_row(statement, &row, 0);
    sqlite3_finalize(statement);

    size_t splitLen = 0;
    if (row.totalChar) {
        char** strSplit = strsplit(row.rows, "\n", &splitLen);
        for (int a = 0; a < splitLen - 1; a++) {
            sprintf(strTemp, "DELETE FROM %s WHERE nama = ?", strSplit[a]);
            sqlite3_prepare_v2(db, strTemp, -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_finalize(statement);
        }
        free(strSplit);
    }

    freeRowBack(&row);
    sqlite3_close(db);

    sb_send_status(e->stream, 200, "OK");

    return SB_RES_OK;
}
