#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

void hapusDaftarBarang(http_event* e) {
    char IDNumber[11], namaBarang[255], strTemp[512];
    SQLRow row = {0};
    sqlite3* db;
    char* errMsg;

    http_get_header(e, "IDNumber", IDNumber, 10);

    if (!IDNumber[0]) {
        http_send_status(e, 403, "Mohon input ID Barang yang benar!");
        return;
    }
    for (int a = 0; a < strlen(IDNumber); a++) if (!isdigit(IDNumber[a])) {
        http_send_status(e, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    sprintf(strTemp, "SELECT nama FROM daftarBarang WHERE rowid=%d;", atoi(IDNumber));

    if ((sqlite3_exec(db, strTemp, RowBack, &row, &errMsg) != SQLITE_OK)) {
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        freeRowBack(&row);
        return;
    }

    if (!row.totalChar) {
        http_send_status(e, 403, "Ada yang salah dengan client! mohon lakukan refresh dengan tekan CTRL + R pada browser");
        return;
    }

    strcpy(namaBarang, row.rows);
    namaBarang[row.totalChar - 1] = '\0';
    freeRowBack(&row);

    sprintf(strTemp, "DELETE FROM daftarBarang WHERE rowid=%d;", atoi(IDNumber));

    if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
    sqlite3_open("database/barangMasuk.db", &db);

    if (sqlite3_exec(db, "SELECT name FROM sqlite_master WHERE type='table'", RowBack, &row, &errMsg) != SQLITE_OK) {
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        freeRowBack(&row);
        sqlite3_close(db);
        return;
    }

    size_t splitLen = 0;
    if (row.totalChar) {
        char** strSplit = strsplit(row.rows, "\n", &splitLen);
        for (int a = 0; a < splitLen - 1; a++) {
            sprintf(strTemp, "DELETE FROM %s WHERE nama='%s'", strSplit[a], namaBarang);
            printf("%s\n", strTemp);
            if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
                http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                printf("[ERROR] Something wrong in SQLite at hapusDaftarBarang.c: %s\n", errMsg);
                sqlite3_free(errMsg);
                freeRowBack(&row);
                sqlite3_close(db);
                return;
            }
        }
        free(strSplit);
    }

    freeRowBack(&row);
    sqlite3_close(db);

    http_send_status(e, 200, "OK");

    return;
}
