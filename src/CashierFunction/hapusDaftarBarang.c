#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

int hapusDaftarBarang(sb_Event* e) {
    char IDNumber[11], namaBarang[255], strTemp[512];
    SQLRow row = {0};
    sqlite3* db;
    char* errMsg;

    sb_get_header(e->stream, "IDNumber", IDNumber, 10);

    if (!IDNumber[0]) {
        sb_send_status(e->stream, 403, "Mohon input ID Barang yang benar!");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(IDNumber); a++) if (!isdigit(IDNumber[a])) {
        sb_send_status(e->stream, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    sprintf(strTemp, "SELECT nama FROM daftarBarang WHERE rowid=%d;", atoi(IDNumber));

    if ((sqlite3_exec(db, strTemp, RowBack, &row, &errMsg) != SQLITE_OK)) {
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        freeRowBack(&row);
        return SB_RES_OK;
    }

    if (!row.totalChar) {
        sb_send_status(e->stream, 403, "Ada yang salah dengan client! mohon lakukan refresh dengan tekan CTRL + R pada browser");
        return SB_RES_OK;
    }

    strcpy(namaBarang, row.rows);
    namaBarang[row.totalChar - 1] = '\0';
    freeRowBack(&row);

    sprintf(strTemp, "DELETE FROM daftarBarang WHERE rowid=%d;", atoi(IDNumber));

    if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sqlite3_close(db);
    sqlite3_open("database/barangMasuk.db", &db);

    if (sqlite3_exec(db, "SELECT name FROM sqlite_master WHERE type='table'", RowBack, &row, &errMsg) != SQLITE_OK) {
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        freeRowBack(&row);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    size_t splitLen = 0;
    if (row.totalChar) {
        char** strSplit = strsplit(row.rows, "\n", &splitLen);
        for (int a = 0; a < splitLen - 1; a++) {
            sprintf(strTemp, "DELETE FROM %s WHERE nama='%s'", strSplit[a], namaBarang);
            printf("%s\n", strTemp);
            if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
                sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
                sqlite3_free(errMsg);
                freeRowBack(&row);
                sqlite3_close(db);
                return SB_RES_OK;
            }
        }
        free(strSplit);
    }

    freeRowBack(&row);
    sqlite3_close(db);

    sb_send_status(e->stream, 200, "OK");

    return SB_RES_OK;
}
