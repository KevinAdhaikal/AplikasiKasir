#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cashierFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"

int tambahBarang(sb_Event* e) {
    char namaBarang[255], jumlahBarang[11], hargaModal[11], barcodeBarang[255], hargaJual[11];
    char* errMsg;
    char strTemp[1024];
    char checkDatabase = 0;
    sqlite3* db;
    int sqlRet;

    sb_get_header(e->stream, "namaBarang", namaBarang, 254);
    sb_get_header(e->stream, "jumlahBarang", jumlahBarang, 10);
    sb_get_header(e->stream, "hargaModal", hargaModal, 10);
    sb_get_header(e->stream, "barcodeBarang", barcodeBarang, 254);
    sb_get_header(e->stream, "hargaJual", hargaJual, 10);

    for (int a = 0; a < strlen(jumlahBarang); a++) if (!isdigit(jumlahBarang[a])) {
        sb_send_status(e->stream, 403, "Jumlah barang tidak berbentuk nomor! Mohon input Jumlah barang yang benar");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(hargaModal); a++) if (!isdigit(hargaModal[a])) {
        sb_send_status(e->stream, 403, "Harga Modal tidak berbentuk nomor! Mohon input Harga barang yang benar");
        return SB_RES_OK;
    }
    for (int a = 0; a < strlen(hargaJual); a++) if (!isdigit(hargaJual[a])) {
        sb_send_status(e->stream, 403, "Harga Jual tidak berbentuk nomor! Mohon input Harga barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    
    if (!barcodeBarang[0])sprintf(strTemp, "SELECT lower(nama) FROM daftarBarang WHERE lower(nama) = lower('%s')", namaBarang, barcodeBarang);
    else sprintf(strTemp, "SELECT lower(nama),barcode FROM daftarBarang WHERE lower(nama) = lower('%s') OR barcode = '%s'", namaBarang, barcodeBarang);
    
    sqlite3_exec(db, strTemp, tableExists, &checkDatabase, &errMsg);
    
    if (checkDatabase) {
        sqlite3_close(db);
        sb_send_status(e->stream, 403, "Nama/Barcode Barang tersebut sudah ada di database! mohon ganti Nama/Barcode barang yang belum ada di Database");
        return SB_RES_OK;
    }

    sprintf(strTemp, "CREATE TABLE IF NOT EXISTS daftarBarang (nama TEXT, jumlah INT, modal INT, jual INT, barcode TEXT); INSERT INTO daftarBarang (nama, jumlah, modal, jual, barcode) values ('%s', %d, %d, %d, '%s')", namaBarang, atoi(jumlahBarang), atoi(hargaModal), atoi(hargaJual), barcodeBarang);
    if (!sqlNormalExec(e, db, strTemp)) return SB_RES_OK;

    sqlite3_close(db);
    sb_send_status(e->stream, 200, "OK");

    return SB_RES_OK;
}