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
    char strTemp[1024];
    sqlite3* db;
    sqlite3_stmt* statement;

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
    
    if (!barcodeBarang[0]) {
        sprintf(strTemp, "SELECT lower(nama) FROM daftarBarang WHERE lower(nama) = lower(?)", namaBarang);
        sqlite3_prepare_v2(db, strTemp, -1, &statement, NULL);
        sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
    }
    else {
        sprintf(strTemp, "SELECT lower(nama),barcode FROM daftarBarang WHERE lower(nama) = lower(?) OR barcode = ?");
        sqlite3_prepare_v2(db, strTemp, -1, &statement, NULL);
        sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
        sqlite3_bind_text(statement, 2, barcodeBarang, -1, SQLITE_STATIC);
    }

    if (sqlite3_step(statement) == SQLITE_ROW) {
        sqlite3_finalize(statement);
        sqlite3_close(db);
        sb_send_status(e->stream, 403, "Nama/Barcode Barang tersebut sudah ada di database! mohon ganti Nama/Barcode barang yang belum ada di Database");
        return SB_RES_OK;
    }

    sqlite3_finalize(statement);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS daftarBarang (nama TEXT, jumlah INT, modal INT, jual INT, barcode TEXT)", 0, 0, NULL);
    sqlite3_prepare_v2(db, "INSERT INTO daftarBarang (nama, jumlah, modal, jual, barcode) values (?, ?, ?, ?, ?)", -1, &statement, NULL);

    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, atoi(jumlahBarang));
    sqlite3_bind_int(statement, 3, atoi(hargaModal));
    sqlite3_bind_int(statement, 4, atoi(hargaJual));
    sqlite3_bind_text(statement, 5, barcodeBarang, -1, SQLITE_STATIC);

    sqlite3_step(statement);
    sqlite3_finalize(statement);

    sqlite3_close(db);
    sb_send_status(e->stream, 200, "OK");

    return SB_RES_OK;
}