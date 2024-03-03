#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "cashierFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"

int tambahBarang(sb_Event* e) {
    char namaBarang[255], jumlahBarang[32], hargaModal[32], barcodeBarang[255], hargaJual[32];
    sqlite3* db;
    sqlite3_stmt* statement;

    sb_get_header(e->stream, "namaBarang", namaBarang, 254);
    sb_get_header(e->stream, "jumlahBarang", jumlahBarang, 31);
    sb_get_header(e->stream, "hargaModal", hargaModal, 31);
    sb_get_header(e->stream, "hargaJual", hargaJual, 31);
    sb_get_header(e->stream, "barcodeBarang", barcodeBarang, 254);

    sqlite3_open("database/daftarBarang.db", &db);
    
    if (!barcodeBarang[0]) {
        sqlite3_prepare_v2(db, "SELECT lower(nama) FROM daftarBarang WHERE lower(nama) = lower(?)", -1, &statement, NULL);
        sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
    }
    else {
        sqlite3_prepare_v2(db, "SELECT lower(nama),barcode FROM daftarBarang WHERE lower(nama) = lower(?) OR barcode = ?", -1, &statement, NULL);
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
    sqlite3_prepare_v2(db, "INSERT INTO daftarBarang (nama, jumlah, modal, jual, barcode) values (?, ?, ?, ?, ?)", -1, &statement, NULL);

    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);

    sqlite3_bind_int64(statement, 2, atoll(jumlahBarang));
    sqlite3_bind_int64(statement, 3, atoll(hargaModal));
    sqlite3_bind_int64(statement, 4, atoll(hargaJual));
    sqlite3_bind_text(statement, 5, barcodeBarang, -1, SQLITE_STATIC);

    sqlite3_step(statement);
    sqlite3_finalize(statement);

    sqlite3_close(db);
    sb_send_status(e->stream, 200, "OK");

    return SB_RES_OK;
}