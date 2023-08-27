#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

void editBarang(http_event* e) {
    char* errMsg;
    char strTemp[1024];
    sqlite3* db;
    SQLRow checkRow = {0};
    char barangID[11], namaBarang[255], jumlahBarang[11], hargaModal[11], hargaJual[11], barcodeBarang[255];

    http_get_header(e, "barangID", barangID, 10);
    http_get_header(e, "namaBarang", namaBarang, 254);
    http_get_header(e, "jumlahBarang", jumlahBarang, 10);
    http_get_header(e, "hargaModal", hargaModal, 10);
    http_get_header(e, "hargaJual", hargaJual, 10);
    http_get_header(e, "barcodeBarang", barcodeBarang, 254);

    for (int a = 0; a < strlen(barangID); a++) if (!isdigit(barangID[a])) {
        http_send_status(e, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return;
    }
    for (int a = 0; a < strlen(jumlahBarang); a++) if (!isdigit(jumlahBarang[a])) {
        http_send_status(e, 403, "Jumlah Barang tidak berbentuk nomor! Mohon input Jumlah Barang yang benar");
        return;
    }
    for (int a = 0; a < strlen(hargaModal); a++) if (!isdigit(hargaModal[a])) {
        http_send_status(e, 403, "Harga Modal tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return;
    }
    for (int a = 0; a < strlen(hargaJual); a++) if (!isdigit(hargaJual[a])) {
        http_send_status(e, 403, "Harga Jual tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return;
    }

    sqlite3_open("database/daftarBarang.db", &db);

    sprintf(strTemp, "SELECT rowid,nama FROM daftarBarang WHERE nama='%s'", namaBarang);

    if (sqlite3_exec(db, strTemp, RowBack, &checkRow, &errMsg)) {
        freeRowBack(&checkRow);
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    sprintf(strTemp, "%s|%s", barangID, namaBarang);
    if (checkRow.rows && !isStr(checkRow.rows, strTemp, 0)) {
        freeRowBack(&checkRow);
        http_send_status(e, 403, "Nama Barang tersebut sudah ada, mohon ganti Nama Barang!");
        sqlite3_close(db);
        return;
    }

    freeRowBack(&checkRow);

    if (barcodeBarang[0]) {
        sprintf(strTemp, "SELECT rowid,barcode FROM daftarBarang WHERE barcode='%s'", barcodeBarang);
        
        if (sqlite3_exec(db, strTemp, RowBack, &checkRow, &errMsg)) {
            freeRowBack(&checkRow);
            http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return;
        }

        sprintf(strTemp, "%s|%s", barangID, barcodeBarang);
        if (checkRow.rows && !isStr(checkRow.rows, strTemp, 0)) {
            freeRowBack(&checkRow);
            http_send_status(e, 403, "Barcode Barang tersebut sudah ada, mohon ganti Nama Barang!");
            sqlite3_close(db);
            return;
        }

        freeRowBack(&checkRow);
    }

    sprintf(strTemp, "UPDATE daftarBarang SET nama='%s', jumlah=%d, modal=%d, jual=%d, barcode='%s' WHERE rowid=%d;", namaBarang, atoi(jumlahBarang), atoi(hargaModal), atoi(hargaJual), barcodeBarang, atoi(barangID));
    
    if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
        freeRowBack(&checkRow);
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    http_send_status(e, 200, "OK");
    sqlite3_close(db);

    return;
}
