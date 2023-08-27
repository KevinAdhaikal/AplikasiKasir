#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cashierFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"

void tambahBarang(http_event* e) {
    char namaBarang[255], jumlahBarang[11], hargaModal[11], barcodeBarang[255], hargaJual[11];
    char* errMsg;
    char strTemp[1024];
    char checkDatabase = 0;
    sqlite3* db;
    int sqlRet;

    http_get_header(e, "namaBarang", namaBarang, 254);
    http_get_header(e, "jumlahBarang", jumlahBarang, 10);
    http_get_header(e, "hargaModal", hargaModal, 10);
    http_get_header(e, "barcodeBarang", barcodeBarang, 254);
    http_get_header(e, "hargaJual", hargaJual, 10);

    for (int a = 0; a < strlen(jumlahBarang); a++) if (!isdigit(jumlahBarang[a])) {
        http_send_status(e, 403, "Jumlah barang tidak berbentuk nomor! Mohon input Jumlah barang yang benar");
        return;
    }
    for (int a = 0; a < strlen(hargaModal); a++) if (!isdigit(hargaModal[a])) {
        http_send_status(e, 403, "Harga Modal tidak berbentuk nomor! Mohon input Harga barang yang benar");
        return;
    }
    for (int a = 0; a < strlen(hargaJual); a++) if (!isdigit(hargaJual[a])) {
        http_send_status(e, 403, "Harga Jual tidak berbentuk nomor! Mohon input Harga barang yang benar");
        return;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    
    if (!barcodeBarang[0])sprintf(strTemp, "SELECT lower(nama) FROM daftarBarang WHERE lower(nama) = lower('%s')", namaBarang, barcodeBarang);
    else sprintf(strTemp, "SELECT lower(nama),barcode FROM daftarBarang WHERE lower(nama) = lower('%s') OR barcode = '%s'", namaBarang, barcodeBarang);
    
    sqlite3_exec(db, strTemp, tableExists, &checkDatabase, &errMsg);
    
    if (checkDatabase) {
        sqlite3_close(db);
        http_send_status(e, 403, "Nama/Barcode Barang tersebut sudah ada di database! mohon ganti Nama/Barcode barang yang belum ada di Database");
        return;
    }

    sprintf(strTemp, "CREATE TABLE IF NOT EXISTS daftarBarang (nama TEXT, jumlah INT, modal INT, jual INT, barcode TEXT); INSERT INTO daftarBarang (nama, jumlah, modal, jual, barcode) values ('%s', %d, %d, %d, '%s')", namaBarang, atoi(jumlahBarang), atoi(hargaModal), atoi(hargaJual), barcodeBarang);
    if (!sqlNormalExec(e, db, strTemp)) return;

    sqlite3_close(db);
    http_send_status(e, 200, "OK");

    return;
}