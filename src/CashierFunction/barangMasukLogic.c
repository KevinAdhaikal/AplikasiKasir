#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../sqliteFunction.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../utils/utils.h"

#include "cashierFunction.h"

void barangMasukLogic(http_event* e) {
    char tanggalBarangMasuk[11];
    char tempString[1024];
    char* errMsg;

    int isTrue = 0;
    sqlite3* db;
    SQLRow rowBack = {0};
    time_t rawtime;
    struct tm *timeinfo;
    char gotoPos = 0;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    switch(http_get_query_to_int(e, "barangMasukArgs")) {
        case 1: {
            http_get_header(e, "tanggalBarangMasuk", tanggalBarangMasuk, 10);
            sqlite3_open("database/barangMasuk.db", &db);

            if (tanggalBarangMasuk[0]) sprintf(tempString, "SELECT * from barangMasuk_%s", tanggalBarangMasuk);
            else sprintf(tempString, "SELECT rowid,* from barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            goto ROWBACK;
            // list barang masuk
            return;
        }
        case 2: {
            char namaBarang[255];
            http_get_header(e, "findBarang", namaBarang, 254);

            if (!namaBarang[0]) {
                http_send_status(e, 403, "Nama/Barcode Barang tidak boleh kosong!");
                return;
            }

            sqlite3_open("database/daftarBarang.db", &db);
            sprintf(tempString, "SELECT nama FROM daftarBarang WHERE nama LIKE '%%%s%%' or barcode='%s';", namaBarang, namaBarang);

            gotoPos = 2;
            goto ROWBACK;
            // check barang masuk
            return;
        }
        case 3: {
            char namaBarang[255];
            char jumlahBarang[11];
            char hargaBarang[11];

            http_get_header(e, "namaBarang", namaBarang, 254);
            http_get_header(e, "jumlahBarang", jumlahBarang, 10);
            http_get_header(e, "hargaBarang", hargaBarang, 11);

            if (!namaBarang[0]) {
                http_send_status(e, 403, "Nama/Barcode barang tidak boleh kosong!");
                return;
            } else if (!jumlahBarang[0]) {
                http_send_status(e, 403, "Jumlah Barang tidak boleh kosong!");
                return;
            } else if (!hargaBarang[0]) {
                http_send_status(e, 403, "Harga Barang tidak boleh kosong!");
                return;
            }

            sqlite3_open("database/barangMasuk.db", &db);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangMasuk_%d_%d_%d (nama TEXT, jumlah INT, harga INT);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) return;
            sprintf(tempString, "INSERT INTO barangMasuk_%d_%d_%d (nama, jumlah, harga) VALUES ('%s', %d, %d)", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, namaBarang, atoi(jumlahBarang), atoi(hargaBarang));
            if (!sqlNormalExec(e, db, tempString)) return;
            sqlite3_close(db);

            sqlite3_open("database/daftarBarang.db", &db);

            sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d where nama='%s'", atoi(jumlahBarang), namaBarang);
            if (!sqlNormalExec(e, db, tempString)) return;

            sqlite3_close(db);
            http_send_status(e, 200, "OK");
            // tambah barang masuk
            return;
        }
        case 4: {
            char namaBarang[255];
            char barangMasukID[11];

            http_get_header(e, "namaBarang", namaBarang, 254);
            http_get_header(e, "tanggalBarangMasuk", tanggalBarangMasuk, 11);
            http_get_header(e, "barangMasukID", barangMasukID, 11);

            if (!namaBarang[0]) {
                http_send_status(e, 403, "Nama/Barcode barang tidak boleh kosong!");
                return;
            }

            if (!barangMasukID[0]) {
                http_send_status(e, 403, "ID Barang Masuk tidak boleh kosong!");
                return;
            }

            sqlite3_open("database/barangMasuk.db", &db);

            if (tanggalBarangMasuk[0]) sprintf(tempString, "SELECT jumlah FROM barangMasuk_%s where rowid='%d'", tanggalBarangMasuk, atoi(barangMasukID));
            else sprintf(tempString, "SELECT jumlah FROM barangMasuk_%d_%d_%d WHERE rowid='%d'", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, atoi(barangMasukID));

            gotoPos = 4;
            goto ROWBACK;

            GOTOPOS4:
            if (tanggalBarangMasuk[0]) sprintf(tempString, "DELETE FROM barangMasuk_%s where rowid=%d", tanggalBarangMasuk, atoi(barangMasukID));
            else sprintf(tempString, "DELETE FROM barangMasuk_%d_%d_%d where rowid=%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, atoi(barangMasukID));

            if (!sqlNormalExec(e, db, tempString)) {
                freeRowBack(&rowBack);
                return;
            }

            sqlite3_close(db);

            sqlite3_open("database/daftarBarang.db", &db);
            rowBack.rows[rowBack.totalChar - 1] = '\0';
            sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah - %d where nama='%s'", atoi(rowBack.rows), namaBarang);
            freeRowBack(&rowBack);
            if (!sqlNormalExec(e, db, tempString)) return;
            sqlite3_close(db);

            http_send_status(e, 200, "OK");
            // hapus barang masuk
            return;
        }
        default: return;
    }

    ROWBACK:
    if (sqlite3_exec(db, tempString, RowBack, &rowBack, &errMsg) != SQLITE_OK) {
        if (!isStr(errMsg, "no such table", 0)) {
            freeRowBack(&rowBack);
            http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at barangMasukLogic.c: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return;
        }
    }

    if (gotoPos == 2) {
        if (!rowBack.totalChar) http_send_status(e, 403, "Nama/Barcode Barang Tidak ada di dalam database!");
        else {
            http_write(e, rowBack.rows, rowBack.totalChar);
            http_send_status(e, 200, "OK");
        }
    }
    else if (gotoPos == 4) goto GOTOPOS4;
    else {
        http_send_status(e, 200, "OK");
        http_write(e, rowBack.rows, rowBack.totalChar);
        sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
        http_write(e, tempString, strlen(tempString));
    }

    sqlite3_close(db);
    freeRowBack(&rowBack);

    return;
}
