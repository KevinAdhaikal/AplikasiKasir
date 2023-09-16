#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

int editBarang(sb_Event* e) {
    char* errMsg;
    char strTemp[1024];
    sqlite3* db;
    SQLRow checkRow = {0};
    char barangID[11], namaBarang[255], jumlahBarang[11], hargaModal[11], hargaJual[11], barcodeBarang[255];

    sb_get_header(e->stream, "barangID", barangID, 10);
    sb_get_header(e->stream, "namaBarang", namaBarang, 254);
    sb_get_header(e->stream, "jumlahBarang", jumlahBarang, 10);
    sb_get_header(e->stream, "hargaModal", hargaModal, 10);
    sb_get_header(e->stream, "hargaJual", hargaJual, 10);
    sb_get_header(e->stream, "barcodeBarang", barcodeBarang, 254);

    for (int a = 0; a < strlen(barangID); a++) if (!isdigit(barangID[a])) {
        sb_send_status(e->stream, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(jumlahBarang); a++) if (!isdigit(jumlahBarang[a])) {
        sb_send_status(e->stream, 403, "Jumlah Barang tidak berbentuk nomor! Mohon input Jumlah Barang yang benar");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(hargaModal); a++) if (!isdigit(hargaModal[a])) {
        sb_send_status(e->stream, 403, "Harga Modal tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(hargaJual); a++) if (!isdigit(hargaJual[a])) {
        sb_send_status(e->stream, 403, "Harga Jual tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);

    sprintf(strTemp, "SELECT rowid,nama FROM daftarBarang WHERE nama='%s'", namaBarang);

    if (sqlite3_exec(db, strTemp, RowBack, &checkRow, &errMsg)) {
        freeRowBack(&checkRow);
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sprintf(strTemp, "%s|%s", barangID, namaBarang);
    if (checkRow.rows && !isStr(checkRow.rows, strTemp, 0)) {
        freeRowBack(&checkRow);
        sb_send_status(e->stream, 403, "Nama Barang tersebut sudah ada, mohon ganti Nama Barang!");
        sqlite3_close(db);
        return SB_RES_OK;
    }

    freeRowBack(&checkRow);

    if (barcodeBarang[0]) {
        sprintf(strTemp, "SELECT rowid,barcode FROM daftarBarang WHERE barcode='%s'", barcodeBarang);
        
        if (sqlite3_exec(db, strTemp, RowBack, &checkRow, &errMsg)) {
            freeRowBack(&checkRow);
            sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return SB_RES_OK;
        }

        sprintf(strTemp, "%s|%s", barangID, barcodeBarang);
        if (checkRow.rows && !isStr(checkRow.rows, strTemp, 0)) {
            freeRowBack(&checkRow);
            sb_send_status(e->stream, 403, "Barcode Barang tersebut sudah ada, mohon ganti Nama Barang!");
            sqlite3_close(db);
            return SB_RES_OK;
        }

        freeRowBack(&checkRow);
    }

    sprintf(strTemp, "UPDATE daftarBarang SET nama='%s', jumlah=%d, modal=%d, jual=%d, barcode='%s' WHERE rowid=%d;", namaBarang, atoi(jumlahBarang), atoi(hargaModal), atoi(hargaJual), barcodeBarang, atoi(barangID));
    
    if (sqlite3_exec(db, strTemp, 0, 0, &errMsg) != SQLITE_OK) {
        freeRowBack(&checkRow);
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at editBarang.c: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sb_send_status(e->stream, 200, "OK");
    sqlite3_close(db);

    return SB_RES_OK;
}
